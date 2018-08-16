// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "oscquery_mirror.hpp"

#include <ossia/network/base/device.hpp>
#include <ossia/network/exceptions.hpp>
#include <ossia/network/osc/detail/osc.hpp>
#include <ossia/network/osc/detail/receiver.hpp>
#include <ossia/network/osc/detail/sender.hpp>
#include <ossia/network/oscquery/detail/http_client.hpp>
#include <ossia/network/oscquery/detail/json_parser.hpp>
#include <ossia/network/oscquery/detail/json_writer.hpp>
#include <ossia/network/oscquery/detail/client.hpp>
#include <ossia/network/oscquery/detail/outbound_visitor.hpp>
#include <ossia/network/osc/detail/osc_receive.hpp>
#include <ossia/network/oscquery/detail/value_to_json.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
namespace ossia
{
namespace oscquery
{

struct http_client_context
{
    std::thread thread;
    asio::io_service context;
    std::shared_ptr<asio::io_service::work> worker;
};
/*
auto wait_for(std::future<void>& fut, std::chrono::milliseconds dur)
{
  using clk = std::chrono::high_resolution_clock;
  std::future_status status = std::future_status::deferred;
  while(dur.count() > 0 || !(status == std::future_status::ready))
  {
    auto start = clk::now();
    auto next = start;
    for(int n = 0; n < 1000; n++)
    {
      next = std::chrono::system_clock::now();
    }
    dur -= std::chrono::duration_cast<std::chrono::milliseconds>(next - start);
    status = fut.wait_for(std::chrono::milliseconds(dur / 8));
    start = std::chrono::system_clock::now();
    dur -= std::chrono::duration_cast<std::chrono::milliseconds>(start - next);
  }
  return status;
}
*/

oscquery_mirror_protocol::oscquery_mirror_protocol(
    std::string host, uint16_t local_osc_port)
  : m_websocketHost{std::move(host)}
  , m_osc_port{local_osc_port}
  , m_http{std::make_unique<http_client_context>()}
{
}

void oscquery_mirror_protocol::cleanup_connections()
{
  try
  {
    m_oscServer->stop();
  }
  catch (...)
  {
    logger().error("Error when stopping osc server");
  }

  if (query_connected())
  {
    try
    {
      query_stop();
    }
    catch (...)
    {
      logger().error("Error when stopping WS server");
    }
  }

  if (m_wsThread.joinable())
  {
    try
    {
      query_stop();
      m_wsThread.join();
    }
    catch (std::exception& e)
    {
      logger().error("Error when stopping WS thread: {}", e.what());
    }
    catch (...)
    {
      logger().error("Error when stopping WS thread");
    }
  }

  try
  {
    m_http->context.stop();
    m_http->worker.reset();
    if (m_http->thread.joinable())
      m_http->thread.join();
  }
  catch (...)
  {
    logger().error("Error when stopping HTTP thread");
  }
}

struct http_responder
{
  oscquery_mirror_protocol& self;
  template<typename T, typename S>
  void on_response(T& req, const S& str)
  {
    if (self.on_WSMessage({}, str))
    {
      req.close();
    }
  }

  template<typename T, typename S>
  void on_error(T& req)
  {
    req.close();
  }
};
void oscquery_mirror_protocol::query_send_message(const std::string& str)
{
  if (!m_useHTTP)
  {
    m_websocketClient->send_message(str);
  }
  else
  {
    auto hrq = std::make_shared<http_get_request>(
        [=](auto& req, const auto& str) {
          bool res = this->on_WSMessage({}, str);
          if (res)
          {
            req.close();
          }
        },
        [=](auto& req) {
          req.close();
        },
        m_http->context, m_websocketHost, m_websocketPort, str);
    hrq->resolve(m_websocketHost, m_websocketPort);
  }
}

void oscquery_mirror_protocol::query_send_message(
    const rapidjson::StringBuffer& str)
{
  if (!m_useHTTP)
  {
    m_websocketClient->send_message(str);
  }
  else
  {
    auto hrq = std::make_shared<http_get_request>(
        [=] (auto& req, const auto& str) {
          bool res = this->on_WSMessage({}, str);
          if (res)
          {
            req.close();
          }
        },
        [=] (auto& req) {
          req.close();
        },
        m_http->context, m_websocketHost, m_websocketPort, str.GetString());
    hrq->resolve(m_websocketHost, m_websocketPort);
  }
}

void oscquery_mirror_protocol::query_send_binary_message(
    const std::string& str)
{
  if (!m_useHTTP)
  {
    m_websocketClient->send_binary_message(str);
  }
}


bool oscquery_mirror_protocol::query_connected()
{
  if (!m_useHTTP)
    return m_websocketClient->connected();
  return true;
}

void oscquery_mirror_protocol::query_stop()
{
  if (!m_useHTTP)
    m_websocketClient->stop();
}

oscquery_mirror_protocol::~oscquery_mirror_protocol()
{
  cleanup_connections();
}

bool oscquery_mirror_protocol::pull(net::parameter_base& address)
{
#if defined(OSSIA_BENCHMARK)
  auto t1 = std::chrono::high_resolution_clock::now();
#endif
  auto fut = pull_async(address);
  auto status = fut.wait_for(std::chrono::milliseconds(3000));

#if defined(OSSIA_BENCHMARK)
  auto t2 = std::chrono::high_resolution_clock::now();
  ossia::logger().info(
      "Time taken: {}",
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
#endif
  return status == std::future_status::ready;
}

std::future<void>
oscquery_mirror_protocol::pull_async(net::parameter_base& address)
{
  std::promise<void> promise;
  auto fut = promise.get_future();
  auto text = address.get_node().osc_address();
  m_getWSPromises.enqueue(get_ws_promise{std::move(promise), text});
  /*
  m_getOSCPromises.insert(
        std::make_pair(
          address.get_node().osc_address(),
          get_promise{std::move(promise), &address}));
  */
  text += detail::query_value();
  query_send_message(text);
  return fut;
}

void oscquery_mirror_protocol::request(net::parameter_base& address)
{
  auto text = address.get_node().osc_address();
  text += detail::query_value();
  query_send_message(text);
}

bool oscquery_mirror_protocol::push(const net::parameter_base& addr)
{
  if (addr.get_access() == ossia::access_mode::GET)
    return false;

  auto val = net::filter_value(addr);
  if (val.valid())
  {
    // Push to server
    auto critical = addr.get_critical();
    if ((!critical || m_useHTTP) && m_oscSender)
    {
      m_oscSender->send(addr, val);
    }
    else
    {
      query_send_binary_message(osc_writer::send_message(addr, val));
    }

    if (m_logger.outbound_listened_logger)
      m_logger.outbound_listened_logger->info("Out: {0}", addr, val);
    return true;
  }
  return false;
}

bool oscquery_mirror_protocol::push_raw(const net::full_parameter_data& addr)
{
  if (addr.get_access() == ossia::access_mode::GET)
    return false;

  auto val = net::filter_value(addr);
  if (val.valid())
  {
    // Push to server
    auto critical = addr.get_critical();
    if ((!critical || m_useHTTP) && m_oscSender)
    {
      m_oscSender->send(addr, val);
    }
    else
    {
      query_send_binary_message(osc_writer::send_message(addr, val));
    }
    return true;
  }
  return false;
}

bool oscquery_mirror_protocol::push_bundle(const std::vector<const ossia::net::parameter_base*>& addresses)
{
  if (!m_useHTTP)
  {
    /* TODO
    json_bundle_builder b;
    for(auto a : addresses)
    {
      const ossia::net::parameter_base& addr = *a;
      ossia::value val = net::filter_value(addr);
      if (val.valid())
      {
        b.add_message(addr, val);
      }
    }

    m_websocketClient->send_binary_message(b.finish());
    */
  }
  return false;
}

bool oscquery_mirror_protocol::push_raw_bundle(const std::vector<ossia::net::full_parameter_data>& addresses)
{
  if (!m_useHTTP)
  {
    /* TODO
    json_bundle_builder b;
    for(const auto& addr : addresses)
    {
      ossia::value val = net::filter_value(addr);
      if (val.valid())
      {
        b.add_message(addr, val);
      }
    }

    m_websocketClient->send_message(b.finish());
    */
  }
  return false;
}


bool oscquery_mirror_protocol::observe(net::parameter_base& address, bool enable)
{
  if (enable)
  {
    auto str = address.get_node().osc_address();

    if (!m_useHTTP)
      query_send_message(json_writer::listen(str));

    m_listening.insert(std::make_pair(std::move(str), &address));
  }
  else
  {
    const auto& str = address.get_node().osc_address();

    if (!m_useHTTP)
      query_send_message(json_writer::ignore(str));

    m_listening.erase(str);
  }
  return true;
}

bool oscquery_mirror_protocol::observe_quietly(
    net::parameter_base& address, bool enable)
{
  if (enable)
    m_listening.insert(
        std::make_pair(address.get_node().osc_address(), &address));
  else
    m_listening.erase(address.get_node().osc_address());

  return true;
}

bool oscquery_mirror_protocol::update(net::node_base& b)
{
  auto fut = update_future(b);
  auto status = fut.wait_for(std::chrono::seconds(3));
  return status == std::future_status::ready;
}

std::future<void> oscquery_mirror_protocol::update_future(net::node_base& b)
{
  m_namespacePromise = std::promise<void>{};
  auto fut = m_namespacePromise.get_future();
  query_send_message(b.osc_address());
  return fut;
}

void oscquery_mirror_protocol::set_device(net::device_base& dev)
{
  m_device = &dev;
  init();
}

void oscquery_mirror_protocol::run_commands()
{
  bool ok = false;
  std::function<void()> cmd;
  do
  {
    ok = m_functionQueue.try_dequeue(cmd);
    if (ok && cmd)
      cmd();
  } while (ok);
}

void oscquery_mirror_protocol::set_command_callback(std::function<void()> cb)
{
  m_commandCallback = std::move(cb);
}

void oscquery_mirror_protocol::request_remove_node(net::node_base& self)
{
  if (auto parent = self.get_parent())
  {
    std::string req;
    req.reserve(64);
    req = parent->osc_address();
    req += '?';
    req += detail::remove_node();
    req += '=';
    req += self.get_name();

    query_send_message(std::move(req));
  }
}

void oscquery_mirror_protocol::set_disconnect_callback(std::function<void()> f)
{
  m_websocketClient->onClose = std::move(f);
}

void oscquery_mirror_protocol::set_fail_callback(std::function<void()> f)
{
  m_websocketClient->onFail = std::move(f);
}

void oscquery_mirror_protocol::init()
{
  m_oscServer = std::make_unique<osc::receiver>(
        m_osc_port,
        [=](const oscpack::ReceivedMessage& m,
            const oscpack::IpEndpointName& ip) {
          this->on_OSCMessage(m, ip);
  });

  m_websocketClient = std::make_unique<websocket_client>(
                        [=] (connection_handler hdl, websocketpp::frame::opcode::value op, std::string& msg) {
    switch (op)
    {
      case websocketpp::frame::opcode::value::TEXT:
        this->on_WSMessage(hdl, msg);
        break;
      case websocketpp::frame::opcode::value::BINARY:
        this->on_BinaryWSMessage(hdl, msg);
        break;
      default:
        break;
    }
  });

  auto port_idx = m_websocketHost.find_last_of(':');
  if (port_idx != std::string::npos)
  {
    m_websocketPort = m_websocketHost.substr(port_idx + 1);
  }
  else
  {
    m_websocketPort = "80";
  }
  if (boost::starts_with(m_websocketHost, "http://"))
  {
    m_useHTTP = true;
    if (port_idx != std::string::npos)
    {
      // for http, host should be only the name, e.g. example.com instead of http://example.com:1234
      m_websocketHost.erase(m_websocketHost.begin() + port_idx, m_websocketHost.end());
      m_websocketHost.erase(m_websocketHost.begin(), m_websocketHost.begin() + 7);
      // m_websocketHost = "127.0.0.1";
    }

    start_http();
  }
  else
  {
    m_wsThread = std::thread([=] {
      try
      {
        m_websocketClient->connect(m_websocketHost);
      }
      catch (...)
      {
        // Websocket does not connect, so let's try http requests
        start_http();
      }
    });

    int n = 0;
    while (!query_connected())
    {
      n++;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      if (n > 1500)
      {
        cleanup_connections();
        throw ossia::connection_error{
            "oscquery_mirror_protocol::oscquery_mirror_protocol: "
            "Could not connect to "
            + m_websocketHost};
      }
    }
  }

  query_send_message("/?HOST_INFO");
  m_oscServer->run();
}

void oscquery_mirror_protocol::start_http()
{
  m_useHTTP = true;

  m_http->worker = std::make_shared<asio::io_service::work>(m_http->context);
  m_http->thread = std::thread([=] { m_http->context.run(); });
}

void oscquery_mirror_protocol::request_add_node(
    net::node_base& parent, const net::parameter_data& dat)
{
  std::string req;
  req.reserve(64);
  req += parent.osc_address();
  req += '?';
  req += detail::add_node();
  req += '=';
  req += dat.name;

  // TODO the other attributes
  query_send_message(std::move(req));
}

void oscquery_mirror_protocol::on_OSCMessage(
    const oscpack::ReceivedMessage& m, const oscpack::IpEndpointName& ip)
{
#if defined(OSSIA_BENCHMARK)
  auto t1 = std::chrono::high_resolution_clock::now();
#endif
  ossia::net::handle_osc_message<true>(m, m_listening, *m_device, m_logger);

#if defined(OSSIA_BENCHMARK)
  auto t2 = std::chrono::high_resolution_clock::now();
  ossia::logger().info(
      "on_OSCMessage : Time taken: {}",
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
#endif
}

std::string to_ip(std::string uri)
{
  uri = boost::algorithm::ierase_first_copy(uri, "http://");
  uri = boost::algorithm::ierase_first_copy(uri, "https://");
  uri = boost::algorithm::ierase_first_copy(uri, "ws://");
  uri = boost::algorithm::ierase_first_copy(uri, "wss://");

  auto pos = uri.find_last_of(':');
  if (pos != std::string::npos)
    uri.erase(pos, uri.size());

  return uri;
}

bool oscquery_mirror_protocol::on_WSMessage(
    oscquery_mirror_protocol::connection_handler, const std::string& message)
{
#if defined(OSSIA_BENCHMARK)
  auto t1 = std::chrono::high_resolution_clock::now();
#endif
  try
  {
    auto data = json_parser::parse(message);
    if (data->IsNull())
    {
      if (m_logger.inbound_logger)
        m_logger.inbound_logger->warn(
            "Invalid WS message received: {}", message);
      return false;
    }
    if (data->IsArray())
    {
      // TODO
    }
    else
    {
      switch (json_parser::message_type(*data))
      {
        case message_type::HostInfo:
        {
          // The ip of the OSC server on the server
          auto info = json_parser::parse_host_info(*data);
          if(!info.osc_ip)
            info.osc_ip = m_websocketHost;
          if(!info.osc_port)
            info.osc_port = boost::lexical_cast<int>(m_websocketPort);
          if(info.osc_transport == host_info::UDP)
          {
            m_oscSender
                = std::make_unique<osc::sender<oscquery::osc_outbound_visitor>>(
                    m_logger, to_ip(m_websocketHost),
                    json_parser::get_port(*data));

            // Send to the server the local receiver port
            if(info.extensions["OSC_STREAMING"])
            {
              query_send_message(
                    json_writer::start_osc_streaming(
                      m_oscServer->port(),
                      m_oscSender->localPort()));
            }
          }
          else
          {
            // TODO
          }

          break;
        }
        case message_type::Namespace:
        {
          json_parser::parse_namespace(m_device->get_root_node(), *data);
          m_namespacePromise.set_value();
          break;
        }
        case message_type::Value:
        {
          // TODO instead just put full path in reply ?
          auto p = m_getWSPromises.peek();
          if (p)
          {
            auto node
                = ossia::net::find_node(m_device->get_root_node(), p->address);

            if (node)
            {
              auto addr = node->get_parameter();
              if (addr)
              {
                json_parser::parse_value(*addr, *data);
                m_device->on_message(*addr);
              }
              else
              {
                m_device->on_unhandled_message(p->address, detail::ReadValue(*data));
              }
            }
            else
            {
              m_device->on_unhandled_message(p->address, oscquery::detail::ReadValue(*data));
            }

            p->promise.set_value();
            m_getWSPromises.pop();
          }

          else // if update from critical param
          {
              for(auto it = data->MemberBegin(), end = data->MemberEnd(); it != end; ++it)
              {
                auto path = get_string_view(it->name);
                if(!path.empty() && path[0] == '/')
                {
                    auto node = ossia::net::find_node(m_device->get_root_node(), path);
                    if(node)
                  {
                    auto addr = node->get_parameter();
                    if (addr)
                    {
                      auto val = addr->value();
                      json_parser::parse_value(*addr, it->value);
                      //val.apply(detail::json_to_value_unchecked{it->value});
                      m_device->on_message(*addr);
                      continue;
                    }
                  }
                }

                m_device->on_unhandled_message(path, detail::ReadValue(it->value));
              }
          }

          break;
        }
        case message_type::PathAdded:
        {
          m_functionQueue.enqueue([ this, doc = std::move(data) ] {
            json_parser::parse_path_added(m_device->get_root_node(), *doc);
          });
          if (m_commandCallback)
            m_commandCallback();
          break;
        }
        case message_type::PathChanged:
        {
          m_functionQueue.enqueue([ this, doc = std::move(data) ] {
            json_parser::parse_path_changed(m_device->get_root_node(), *doc);
          });
          if (m_commandCallback)
            m_commandCallback();
          break;
        }
        case message_type::PathRemoved:
        {
          m_functionQueue.enqueue([ this, doc = std::move(data) ] {
            json_parser::parse_path_removed(m_device->get_root_node(), *doc);
          });
          if (m_commandCallback)
            m_commandCallback();
          break;
        }
        case message_type::AttributesChanged:
        {
          m_functionQueue.enqueue([ this, doc = std::move(data) ] {
            json_parser::parse_attributes_changed(
                m_device->get_root_node(), *doc);
          });
          if (m_commandCallback)
            m_commandCallback();
          break;
        }
        default:
          break;
      }
    }
  }
  catch (std::exception& e)
  {
    if (m_logger.inbound_logger)
      m_logger.inbound_logger->warn(
          "Error while parsing: {} ==> {}", e.what(), message);
    return false;
  }

  if (m_logger.inbound_logger)
    m_logger.inbound_logger->info("WS In: {}", message);
#if defined(OSSIA_BENCHMARK)
  auto t2 = std::chrono::high_resolution_clock::now();
  ossia::logger().info(
      "on_WSMessage : Time taken: {}",
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
#endif
  return true;
}

bool oscquery_mirror_protocol::on_BinaryWSMessage(oscquery_mirror_protocol::connection_handler hdl, const std::string& message)
{
  auto handler = [=](const oscpack::ReceivedMessage& m,
      const oscpack::IpEndpointName& ip) {
    this->on_OSCMessage(m, ip);
  };
  osc::listener<decltype(handler)> h{handler};
  // TODO use proper ip / port
  h.ProcessPacket(message.data(), message.size(), {});

  return {};
}
}
}
