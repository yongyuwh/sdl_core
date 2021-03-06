/*

 Copyright (c) 2013, Ford Motor Company
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following
 disclaimer in the documentation and/or other materials provided with the
 distribution.

 Neither the name of the Ford Motor Company nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */
#include "application_manager/commands/mobile/send_location_request.h"
#include "application_manager/application_manager_impl.h"
#include "application_manager/message_helper.h"

namespace application_manager {

namespace commands {

SendLocationRequest::SendLocationRequest(const MessageSharedPtr& message)
 : CommandRequestImpl(message) {
}

SendLocationRequest::~SendLocationRequest() {
}

void SendLocationRequest::Run() {
  LOG4CXX_INFO(logger_, "SendLocationRequest::Run");

  ApplicationSharedPtr app = application_manager::ApplicationManagerImpl::instance()
      ->application(connection_key());

  if (!app) {
    LOG4CXX_ERROR_EXT(
        logger_, "An application " << app->name() << " is not registered.");
    SendResponse(false, mobile_apis::Result::APPLICATION_NOT_REGISTERED);
    return;
  }

  if (IsWhiteSpaceExist()) {
    LOG4CXX_ERROR(logger_, "Strings contain invalid characters");
    SendResponse(false, mobile_apis::Result::INVALID_DATA);
    return;
  }

  if ((*message_)[strings::msg_params].keyExists(strings::location_image)) {
    mobile_apis::Result::eType verification_result =
        mobile_apis::Result::SUCCESS;
    verification_result = MessageHelper::VerifyImage(
        (*message_)[strings::msg_params][strings::location_image], app);
    if (mobile_apis::Result::SUCCESS != verification_result) {
      LOG4CXX_ERROR(logger_, "VerifyImage INVALID_DATA!");
      SendResponse(false, verification_result);
      return;
    }
  }

  SendHMIRequest(hmi_apis::FunctionID::Navigation_SendLocation,
                 &((*message_)[strings::msg_params]),
                   true);
}

void SendLocationRequest::on_event(const event_engine::Event& event) {
  LOG4CXX_INFO(logger_, "SendLocationRquest::on_event");
  const smart_objects::SmartObject& message = event.smart_object();
  switch (event.id()) {
    case hmi_apis::FunctionID::Navigation_SendLocation: {
      LOG4CXX_INFO(logger_, "Received Navigation_SendLocation event");
      mobile_apis::Result::eType result_code = GetMobileResultCode(
          static_cast<hmi_apis::Common_Result::eType>(
              message[strings::params][hmi_response::code].asUInt()));
      bool result = mobile_apis::Result::SUCCESS == result_code;
      SendResponse(result, result_code, NULL, &(message[strings::msg_params]));
      break;
    }
    default: {
      LOG4CXX_ERROR(logger_,"Received unknown event" << event.id());
      break;
    }
  }
}

bool SendLocationRequest::IsWhiteSpaceExist() {
  LOG4CXX_INFO(logger_, "SendLocationRquest::IsWhiteSpaceExist");
  const char* str;
  const smart_objects::SmartObject& msg_params =
      (*message_)[strings::msg_params];

  if (msg_params.keyExists(strings::location_name)) {
    str = msg_params[strings::location_name].asCharArray();
    if (!CheckSyntax(str)) {
      LOG4CXX_ERROR(logger_,
                    "parameter locationName contains invalid character");
      return true;
    }
  }

  if (msg_params.keyExists(strings::location_description)) {
    str = msg_params[strings::location_description].asCharArray();
    if (!CheckSyntax(str)) {
      LOG4CXX_ERROR(logger_,
                    "parameter locationDescription contains invalid character");
      return true;
    }
  }

  if (msg_params.keyExists(strings::address_lines)) {
    const smart_objects::SmartArray* al_array =
        msg_params[strings::address_lines].asArray();
    smart_objects::SmartArray::const_iterator it_al = al_array->begin();
    smart_objects::SmartArray::const_iterator it_al_end = al_array->end();
    for (; it_al != it_al_end; ++it_al) {
      str = (*it_al).asCharArray();
      if(!CheckSyntax(str)) {
        LOG4CXX_ERROR(logger_,
                      "parameter address_lines contains invalid character");
        return true;
      }
    }
  }

  if (msg_params.keyExists(strings::phone_number)) {
    str = msg_params[strings::phone_number].asCharArray();
    if (!CheckSyntax(str)) {
      LOG4CXX_ERROR(logger_,
                    "parameter phoneNumber contains invalid character");
      return true;
    }
  }

  if (msg_params.keyExists(strings::location_image)) {
    str = msg_params[strings::location_image][strings::value].asCharArray();
    if (!CheckSyntax(str)) {
      LOG4CXX_ERROR(logger_,
                          "parameter value in locationImage contains invalid character");
      return true;
    }
  }

  return false;
}

}  // namespace commands

}  // namespace application_manager
