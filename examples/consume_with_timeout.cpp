/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MIT
 *
 * Portions created by VMware are Copyright (c) 2007-2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Portions created by Tony Garnock-Jones are Copyright (c) 2009-2010
 * VMware, Inc. and Tony Garnock-Jones. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ***** END LICENSE BLOCK *****
 */


#include <SimpleAmqpClient.h>
#include <SimpleAmqpClient/SimpleRpcClient.h>
#include <SimpleAmqpClient/SimpleRpcServer.h>

#define BOOST_ALL_NO_LIB
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <stdlib.h>

using namespace AmqpClient;

class thread_body : boost::noncopyable
{
	public:
		thread_body()
		{
			char* szBroker = getenv("AMQP_BROKER");
			Channel::ptr_t channel;
			if (szBroker != NULL)
				channel = Channel::Create(szBroker);
			else
				channel = Channel::Create();


			server = SimpleRpcServer::Create(channel, "consume_timeout_test");

			m_thread =
				boost::make_shared<boost::thread>(boost::bind(&thread_body::run,
							this));
		}

		virtual ~thread_body()
		{
			std::cout << "Is joinable " << m_thread->joinable() << "inter reqd: " << m_thread->interruption_requested() << std::endl;
			m_thread->interrupt();
			std::cout << "Is joinable " << m_thread->joinable() << "inter reqd: " << m_thread->interruption_requested() << std::endl;
			m_thread->join();
		}

		void run()
		{
			while (!boost::this_thread::interruption_requested())
			{
				std::cout << "Waiting for message... " << std::flush;
				BasicMessage::ptr_t message;
				if (server->GetNextIncomingMessage(message, 1))
				{
					std::cout << "message received.\n";
          server->RespondToMessage(message, "this is a response");
				}
				else
				{
					std::cout << "message not received.\n";
				}
			}
		}

		SimpleRpcServer::ptr_t server;
		boost::shared_ptr<boost::thread> m_thread;

};
int main()
{
	boost::shared_ptr<thread_body> body = boost::make_shared<thread_body>();

	boost::this_thread::sleep(boost::posix_time::seconds(1));

  char* szBroker = getenv("AMQP_BROKER");
  Channel::ptr_t channel;
  if (szBroker != NULL)
    channel = Channel::Create(szBroker);
  else
    channel = Channel::Create();

  SimpleRpcClient::ptr_t client = SimpleRpcClient::Create(channel, "consume_timeout_test");
  std::string str = client->Call("Here is my message");
  std::cout << "Got response: " << str << std::endl;

	boost::this_thread::sleep(boost::posix_time::seconds(2));

	return 0;
}
