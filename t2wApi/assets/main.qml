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

import bb.cascades 1.2

Page {
  ScrollView {
    Container {
        topPadding: 30
        rightPadding: topPadding
        leftPadding: topPadding
        bottomPadding: topPadding
        Button {
            id: resendNotification
            text: "Resend Authorization"
            onClicked: {
                _app.resendAuthorization();
            }
        }
        Button {
            id: stopHeadless
            text: "Stop Headless"
            onClicked: {
                _app.stopHeadless();
                resendNotification.enabled = false;
                stopHeadless.enabled = false;
                startHeadless.enabled = true;
            }
        }
        Button {
            id: startHeadless
            text: "Start Headless"
            enabled: false
            onClicked: {
                _app.startHeadless();
                resendNotification.enabled = true;
                stopHeadless.enabled = true;
                startHeadless.enabled = false;
            }
        }
        Container {
            topPadding: 100
            Label {
                text: "Talk2Watch authorization and action creation only occurs the first time you run the app, or if you click 'Resend Authorization'. Click 'Reset and Quit' if you would like to restart the app like if it was freshly installed."
                multiline: true
                textStyle.textAlign: TextAlign.Justify
            }
            Button {
                id: resetAndQuit
                text: "Reset and Quit"
                onClicked: {
                    if (stopHeadless.enabled) {
                        _app.stopHeadless();                        
                    }
                    _app.resetAndQuit();
                }
            }
        }
    }
  }
}