/*
 * Copyright (c) 2013 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "applicationui.hpp"

// includes for T2W API
#include "Talk2WatchInterface.h"
#include "UdpModule.h"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/system/InvokeManager>

using namespace bb::cascades;
using namespace bb::system;

ApplicationUI::ApplicationUI(Application *app):
		QObject(app),
		m_translator(new QTranslator(this)),
		m_localeHandler(new LocaleHandler(this)),
		m_invokeManager(new InvokeManager(this)),
		settings("Roger Leblanc", "t2wApi_HL")  // settings should have the same values as the Headless part
{
	// Sync the settings at startup to make sure everything's sync with HL part
	settings.sync();

	// Set "Not found" as T2W version value, this will be changed later if T2W is found
	QString defaultValue = "Not found";
	settings.setValue("t2wVersion", defaultValue);

	// Create new Talk2WatchInterface object and connect to signal
	// This is only needed to check for T2W version, as everything else will be handle by HL part
    t2w = new Talk2WatchInterface(this);
    connect(t2w, SIGNAL(transmissionReady()), this, SLOT(onTransmissionReady()));

	// prepare the localization
	if (!QObject::connect(m_localeHandler, SIGNAL(systemLanguageChanged()),
			this, SLOT(onSystemLanguageChanged()))) {
		// This is an abnormal situation! Something went wrong!
		// Add own code to recover here
		qWarning() << "Recovering from a failed connect()";
	}

	// initial load
	onSystemLanguageChanged();

	// Create scene document from main.qml asset, the parent is set
	// to ensure the document gets destroyed properly at shut down.
	QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

	// Make app available to the qml.
	qml->setContextProperty("_app", this);

	// Create root object for the UI
	AbstractPane *root = qml->createRootObject<AbstractPane>();

	// Set created root object as the application scene
	app->setScene(root);
}

void ApplicationUI::onSystemLanguageChanged() {
	QCoreApplication::instance()->removeTranslator(m_translator);
	// Initiate, load and install the application translation files.
	QString locale_string = QLocale().name();
	QString file_name = QString("t2wApi_%1").arg(locale_string);
	if (m_translator->load(file_name, "app/native/qm")) {
		QCoreApplication::instance()->installTranslator(m_translator);
	}
}

void ApplicationUI::resendAuthorization() {
	if (t2w->isTalk2WatchProServiceInstalled() || t2w->isTalk2WatchProInstalled()) {
		// This is used to call HL part to refresh T2W authorization and create actions
		InvokeRequest request;
		request.setTarget("com.example.t2wApiService");
		request.setAction("com.example.t2wApiService.RESET");
		m_invokeManager->invoke(request);
		Application::instance()->minimize();
	}
	// If Talk2Watch Free is detected, warn the user that this option is only for Pro version
	else if (t2w->isTalk2WatchInstalled())
		t2w->sendSms("Talk2Watch Pro not found", "This option is limited to Talk2Watch Pro users.");
	else
		qDebug() << "No Talk2Watch found";
}

void ApplicationUI::onTransmissionReady()
{
	// Save T2W version to QSettings so HL part can access it
	if (t2w->isTalk2WatchProServiceInstalled() || t2w->isTalk2WatchProInstalled())
		settings.setValue("t2wVersion", "Pro");
	else
		settings.setValue("t2wVersion", "Free");

	// Starts the headless part if T2W was found
    bb::system::InvokeRequest request;
	request.setTarget("com.example.t2wApiService");
	request.setAction("bb.action.system.STARTED");
	request.setMimeType("application/vnd.blackberry.system.event.STARTED");
	m_invokeManager->invoke(request);
}

void ApplicationUI::stopHeadless()
{
	// Send a request to stop headless part
	InvokeRequest request;
	request.setTarget("com.example.t2wApiService");
	request.setAction("bb.action.STOP");
	m_invokeManager->invoke(request);
}

void ApplicationUI::startHeadless()
{
	// Send a request to start headless part
	InvokeRequest request;
	request.setTarget("com.example.t2wApiService");
	request.setAction("bb.action.system.STARTED");
	request.setMimeType("application/vnd.blackberry.system.event.STARTED");
	m_invokeManager->invoke(request);
}

void ApplicationUI::resetAndQuit()
{
	// This allows to restart the app like if it was newly installed
	// So, the T2W authorization and action creation will occur on next startup
	settings.clear();
	Application::instance()->quit();
}
