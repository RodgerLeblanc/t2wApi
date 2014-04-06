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

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>
#include <QSettings>

namespace bb {
	namespace cascades {
		class Application;
		class LocaleHandler;
	}
	namespace system {
		class InvokeManager;
	}
}

class QTranslator;
class Talk2WatchInterface;
class UdpModule;

/*!
 * @brief Application object
 *
 *
 */

class ApplicationUI: public QObject {
	Q_OBJECT
public:
	ApplicationUI(bb::cascades::Application *app);
	virtual ~ApplicationUI() { }

	Q_INVOKABLE	void resendAuthorization();
	Q_INVOKABLE void stopHeadless();
	Q_INVOKABLE void startHeadless();
	Q_INVOKABLE void resetAndQuit();

private slots:
	void onSystemLanguageChanged();
    void onTransmissionReady();

private:
	QTranslator* m_translator;
	bb::cascades::LocaleHandler* m_localeHandler;
	bb::system::InvokeManager* m_invokeManager;

	Talk2WatchInterface* t2w;
    UdpModule *udp;

	QSettings settings;
};

#endif /* ApplicationUI_HPP_ */
