// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <QtTest>
#include <ossia/ossia.hpp>
#include <ossia/context.hpp>
#include <ossia/network/oscquery/detail/json_parser.hpp>
#include <ossia/network/oscquery/detail/json_writer.hpp>
#include <iostream>
#include <ossia/network/oscquery/oscquery_mirror.hpp>
#include <ossia/network/oscquery/oscquery_server.hpp>
#include <ossia/network/oscquery/detail/http_query_parser.hpp>
#include "TestUtils.hpp"

using namespace ossia;
using namespace ossia::net;
class HttpTest : public QObject
{
    Q_OBJECT

private:
    void test_http()
    {
      {
        auto res = ossia::oscquery::parse_http_methods_encoded("foo=bar");
        QVERIFY(res.size() == 1);
        QVERIFY(res.find("foo") != res.end());
        QVERIFY(res.find("foo").value() == "bar");
      }

      {
        auto res = ossia::oscquery::parse_http_methods_encoded("LISTEN=1");
        QVERIFY(res.size() == 1);
        QVERIFY(res.find("LISTEN") != res.end());
        QVERIFY(res.find("LISTEN").value() == "1");
      }

      {
        auto res = ossia::oscquery::parse_http_methods_encoded("VALUE");
        QVERIFY(res.size() == 1);
        QVERIFY(res.find("VALUE") != res.end());
        QVERIFY(res.find("VALUE").value() == "");
      }

      {
        auto res = ossia::oscquery::parse_http_methods_encoded("foo=bar&baz=blu");
        QVERIFY(res.size() == 2);
        QVERIFY(res.find("foo") != res.end());
        QVERIFY(res.find("foo").value() == "bar");
        QVERIFY(res.find("baz") != res.end());
        QVERIFY(res.find("baz").value() == "blu");
      }
    }

    void test_parse()
    {
      generic_device dev{std::make_unique<multiplex_protocol>(), "A"};

      // Create a node
      {
        auto& n = find_or_create_node(dev, "/main");
        auto a = n.create_parameter(ossia::val_type::FLOAT);

        a->push_value(6);

        n.set(access_mode_attribute{}, access_mode::GET);
        n.set(bounding_mode_attribute{}, bounding_mode::FOLD);
        n.set(domain_attribute{}, make_domain(-10., 10.));
        n.set(default_value_attribute{}, ossia::value(0.));
        n.set(tags_attribute{}, tags{"fancy", "wow", "1234"});
        n.set(refresh_rate_attribute{}, 100);
        n.set(value_step_size_attribute{}, 0.5);
        n.set(repetition_filter_attribute{}, repetition_filter::ON);
        n.set(critical_attribute{}, true);
        n.set(unit_attribute{}, meter_per_second_u{});
        n.set(priority_attribute{}, 50);
        n.set(description_attribute{}, "Such a fancy node?! Incredible! すごい!!");
        n.set(extended_type_attribute{}, "custom");
        n.set(app_name_attribute{}, "AppName");
        n.set(app_version_attribute{}, "1.0.0");
        n.set(app_creator_attribute{}, "Lelouch vi Brittania");
      }

      // Node -> json
      auto str = ossia::oscquery::json_writer::query_namespace(dev);
      std::cerr << str.GetString() << std::endl;

      // Clear the device
      dev.clear_children();

      // Parse json
      rapidjson::Document doc;
      doc.Parse( str.GetString() );

      // Json -> node
      ossia::oscquery::json_parser::parse_namespace(dev, doc);

      {
        auto node = find_node(dev, "/main");
        QVERIFY(node);
        auto& n = *node;

        QVERIFY((bool)get_access_mode(n));
        QCOMPARE(*get_access_mode(n), access_mode::GET);

        QVERIFY((bool)get_bounding_mode(n));
        QCOMPARE(*get_bounding_mode(n), bounding_mode::FOLD);

        QVERIFY((bool)get_domain(n));
        QCOMPARE(get_domain(n), make_domain(-10., 10.));

        QVERIFY((bool)get_default_value(n));
        QCOMPARE(*get_default_value(n), ossia::value(0));

        tags the_tags{"fancy", "wow", "1234"};
        QVERIFY((bool)get_tags(n));
        QCOMPARE(*get_tags(n), the_tags);

        QVERIFY((bool)get_refresh_rate(n));
        QCOMPARE(*get_refresh_rate(n), 100);

        QVERIFY((bool)get_value_step_size(n));
        QCOMPARE(*get_value_step_size(n), 0.5);

        QVERIFY(get_repetition_filter(n));
        QCOMPARE(get_repetition_filter(n), repetition_filter::ON);

        QVERIFY(get_critical(n));

        QVERIFY((bool)get_unit(n));
        QCOMPARE(get_unit(n), ossia::unit_t(meter_per_second_u{}));

        QVERIFY((bool)get_priority(n));
        QCOMPARE(*get_priority(n), 50.f);

        QVERIFY((bool)get_description(n));
        QCOMPARE(*get_description(n), std::string("Such a fancy node?! Incredible! すごい!!"));

        QVERIFY((bool)get_extended_type(n));
        QCOMPARE(*get_extended_type(n), std::string("custom"));

        QVERIFY((bool)get_app_name(n));
        QCOMPARE(*get_app_name(n), std::string("AppName"));

        QVERIFY((bool)get_app_version(n));
        QCOMPARE(*get_app_version(n), std::string("1.0.0"));

        QVERIFY((bool)get_app_creator(n));
        QCOMPARE(*get_app_creator(n), std::string("Lelouch vi Brittania"));
      }
    }
};


QTEST_APPLESS_MAIN(HttpTest)

#include "HttpTest.moc"

