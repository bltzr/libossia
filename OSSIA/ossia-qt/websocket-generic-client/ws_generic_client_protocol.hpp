#pragma once
#include <ossia/network/base/protocol.hpp>
#include <wobjectdefs.h>
#include <ossia-qt/websocket-generic-client/ws_generic_client_parameter.hpp>

#include <QByteArray>
#include <QJSValue>
#include <QList>
#include <QObject>

class QQmlEngine;
class QQmlComponent;
class QWebSocket;
class QNetworkReply;

namespace ossia
{
namespace net
{
class ws_generic_client_device;
class ws_generic_client_node;
class ws_generic_client_parameter;

class OSSIA_EXPORT ws_generic_client_protocol final
    : public QObject,
      public ossia::net::protocol_base
{
  W_OBJECT(ws_generic_client_protocol)

public:
  ws_generic_client_protocol(const QString& addr, QByteArray code);

  ws_generic_client_protocol(const ws_generic_client_protocol&) = delete;
  ws_generic_client_protocol(ws_generic_client_protocol&&) = delete;
  ws_generic_client_protocol& operator=(const ws_generic_client_protocol&)
      = delete;
  ws_generic_client_protocol& operator=(ws_generic_client_protocol&&) = delete;

  ~ws_generic_client_protocol();

  bool update(ossia::net::node_base& node_base) override;

  bool pull(ossia::net::parameter_base& parameter_base) override;

  bool push(const ossia::net::parameter_base& parameter_base) override;
  bool push_raw(const ossia::net::full_parameter_data& parameter_base) override;

  bool observe(ossia::net::parameter_base& parameter_base, bool enable) override;

  void set_device(ossia::net::device_base& dev) override;

  static ws_generic_client_parameter_data read_data(const QJSValue& js)
  {
    return js;
  }
public:
  void sig_push(const ws_generic_client_parameter* arg_1) W_SIGNAL(sig_push, arg_1);

private:
  void slot_push(const ws_generic_client_parameter*); W_SLOT(slot_push);

private:
  void apply_reply(QJSValue);

  QQmlEngine* mEngine{};
  QQmlComponent* mComponent{};
  QObject* mItem{};

  QWebSocket* mWebsocket{};

  QByteArray mCode;
  ws_generic_client_device* mDevice{};
  QList<std::pair<QNetworkReply*, const ws_generic_client_parameter*>> mReplies;
};
}
}

Q_DECLARE_METATYPE(ossia::net::ws_generic_client_parameter*)
W_REGISTER_ARGTYPE(ossia::net::ws_generic_client_parameter*)
W_REGISTER_ARGTYPE(const ossia::net::ws_generic_client_parameter*)
