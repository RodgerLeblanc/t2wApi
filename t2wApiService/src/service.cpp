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


#include "service.hpp"

// includes for T2W API
#include "Talk2WatchInterface.h"
#include "UdpModule.h"

#include <bb/Application>
#include <bb/ApplicationInfo>
#include <bb/platform/Notification>
#include <bb/platform/NotificationDefaultApplicationSettings>
#include <bb/system/InvokeManager>

using namespace bb::platform;
using namespace bb::system;

Service::Service(bb::Application * app)	:
		QObject(app),
		// Notification (toast and hub) and InvokeManager (to call UI part) initiator
		m_notify(new Notification(this)),
		m_invokeManager(new InvokeManager(this)),
		settings("Roger Leblanc", "t2wApi_HL") // settings should have the same values as the UI part
{
	// Sync the settings at startup to make sure everything's sync with HL part
	settings.sync();

	// Create new Talk2WatchInterface object and connect to signal
	t2w = new Talk2WatchInterface(this);
    connect(t2w, SIGNAL(transmissionReady()), this, SLOT(onTransmissionReady()));

    // Create UdpModule object, open a UDP port for communicating with T2W and connect to signal
    udp = new UdpModule(this);
    udp->listenOnPort(9712); // this number should be changed to a random unused port
    connect(udp, SIGNAL(reveivedData(QString)), this, SLOT(onUdpDataReceived(QString)));

    // Allow toast notification
    NotificationDefaultApplicationSettings notificationSettings;
	notificationSettings.setPreview(NotificationPriorityPolicy::Allow);
	notificationSettings.apply();

	// Handle invocation from UI part by connecting to signal
	m_invokeManager->connect(m_invokeManager, SIGNAL(invoked(const bb::system::InvokeRequest&)),
            this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));
}

void Service::handleInvoke(const bb::system::InvokeRequest & request) {
	// Handle invocation :
	// On RESET, refresh T2W authorization and create actions
	// On STOP, close headless part
	if (request.action().compare("com.example.t2wApiService.RESET") == 0) {
		authorizeAppWithT2w();
	}
	if (request.action().compare("bb.action.STOP") == 0) {
		t2w->sendSms("Headless says goodbye!", "Headless is now shut down");
		bb::Application::instance()->quit();
	}
}

void Service::onNotifyDemand() {
	// Notify toast and hub + invoke UI part
	bb::system::InvokeRequest request;
	request.setTarget("com.example.t2wApi");
	request.setAction("bb.action.START");
	m_notify->setInvokeRequest(request);

	Notification::clearEffectsForAll();
	Notification::deleteAllFromInbox();

	m_notify->notify();
}

void Service::onTransmissionReady()
{
	qDebug() << "onTransmissionReady -- HL";
	// Aknowledge the user that the connection is made
	t2w->sendSms("Connected to Talk2Watch", "The app is now connected to Talk2Watch and can send messages to the watch. Hit ScriptMode for more options.");

	// Sync the settings to make sure everything's sync with HL part
	settings.sync();

	bool defaultValue = true;

	// Authorize this app to send action to Talk2Watch. This need to be done only once.
	if (settings.value("runForTheFirstTime", defaultValue).toBool()) {
		authorizeAppWithT2w();
		settings.setValue("runForTheFirstTime", false);
	}
}

void Service::authorizeAppWithT2w()
{
	// Sync the settings to make sure everything's sync with HL part
	settings.sync();

	QString defaultValue = "Free";

	// T2W authorization request --- Limited to Pro version
	if (settings.value("t2wVersion", defaultValue).toString() == "Pro") {
		QString appName = "t2wApi_HL";

		// Retrieve app version
		bb::ApplicationInfo appInfo;
    	QString version = appInfo.version() ;

		QString uuid = "614bf149-5f54-4c61-8df0-4ec1ef94dea2";  // Randomly generated online
		QString t2wAuthUdpPort = "9712";  // This should be set to the same value you used when initiating udpModule Object
		QString description = "Talk2Watch API sample headless app";
		t2w->setAppValues(appName, version, uuid, "UDP", t2wAuthUdpPort, description);  // Aknowledge T2W of the app infos
		t2w->sendAppAuthorizationRequest();  // Send authorization request to T2W
	}
}

void Service::onUdpDataReceived(QString _data)
{
	// This is called after T2W authorize the app
	if(_data=="AUTH_SUCCESS") {
		qDebug() << "Auth_Success!!!";

		title << "Send message" << "Send toast"; // << "And on, and on, and on..."
		command << "T2WAPI_HL_FIRST_ACTION" << "T2WAPI_HL_SECOND_ACTION"; // Custom command names
		description << "Send a message to the watch" << "Send a toast and hub notification";

		// Create the first action
		if (title.size() > 0)
			t2w->createAction(title[0], command[0], description[0]);
		return;
	}

	// This is called after an action was successfully created
	if (_data=="CREATE_ACTION_SUCCESS") {
		qDebug() << "Create_Action_success";
		// Remove last action. If there's another action in the list, create it, otherwise return
		title.removeFirst();
		command.removeFirst();
		description.removeFirst();
		if (title.size() > 0)
			t2w->createAction(title[0], command[0], description[0]);
		else {
			// Send a message to the watch to let the user know that everything was done.
			t2w->sendSms("Authorization granted", "Your app was successfully authorized with T2W, all actions were created.");
			return;
		}
	}

	// This is called when the user triggers the first action in ScriptMode
	// Send a message to the watch
	if (_data=="T2WAPI_HL_FIRST_ACTION") {
		t2w->sendSms("First action triggered", "You have fired the first action in ScriptMode");
	}

	// This is called when the user triggers the second action in ScriptMode
	// Send a toast + hub notification
	if (_data=="T2WAPI_HL_SECOND_ACTION") {
		m_notify->setTitle("t2wApi Service");
		m_notify->setBody("User triggered ScriptMode second action");
		onNotifyDemand();
	}
}
