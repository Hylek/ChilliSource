//
//  The MIT License (MIT)
//
//  Copyright (c) 2017 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#ifdef CS_TARGETPLATFORM_RPI

#ifndef _CSBACKEND_PLATFORM_RPI_INPUT_KEYBOARD_KEYBOARD_H_
#define _CSBACKEND_PLATFORM_RPI_INPUT_KEYBOARD_KEYBOARD_H_

#include <ChilliSource/Input/Keyboard/Keyboard.h>
#include <ChilliSource/Input/Keyboard/KeyCode.h>

namespace CSBackend
{
	namespace RPi
	{
        /// A system for receiving input from the current keyboard via X11
        ///
        /// The methods in this class are not thread-safe and must be
        /// accessed from the main thread.
        ///
		class Keyboard final : public ChilliSource::Keyboard
		{
		public:
            CS_DECLARE_NAMEDTYPE(Keyboard);

			/// Queries whether or not this system implements the
			/// interface with the given Id.
			///
			/// @param interfaceId
			///		The id of the interface that we need to check if this system implements
			///
			/// @retrun TRUE if the system 'IsA' interface
			///
			bool IsA(ChilliSource::InterfaceIDType interfaceId) const override;
			/// @param code
			///		Code of the key to check if down
			///
			/// @return TRUE if the given key is currently down.
			/// This is unbuffered so only checks the status of the key at the moment the function is called
			///
			bool IsKeyDown(ChilliSource::KeyCode code) const override;

			/// This event is guaranteed and should be used for low
			/// frequency events such as catching a confirm enter press.
			/// The polling "IsDown" method should be used for realtime
			/// events such as moving characters on arrow press, etc.
			///
			/// The event also returns the current state of the modifier
			/// keys (Ctrl, Alt, Shift, etc.)
			///
			/// @return Event to register for key presses
			///
			ChilliSource::IConnectableEvent<KeyPressedDelegate>& GetKeyPressedEvent() override;

			/// This event is guaranteed and should be used for low
			/// frequency events. The polling "IsUp" method should be
			/// used for realtime events.
			///
			/// @return Event to register for key releases
			///
			ChilliSource::IConnectableEvent<KeyReleasedDelegate>& GetKeyReleasedEvent() override;

        private:

			friend ChilliSource::KeyboardUPtr ChilliSource::Keyboard::Create();

			///
			Keyboard() = default;

		private:

			ChilliSource::Event<KeyPressedDelegate> m_keyPressedEvent;
			ChilliSource::Event<KeyReleasedDelegate> m_keyReleasedEvent;
		};
	}
}

#endif

#endif
