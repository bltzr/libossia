#pragma once
#include <ossia/dataflow/graph_node.hpp>
#include <ossia/dataflow/port.hpp>
#include <ossia/detail/pod_vector.hpp>

namespace ossia::nodes
{
class step final :
    public ossia::graph_node
{
public:
  step()
  {
    m_outlets.push_back(ossia::make_outlet<ossia::value_port>());
  }

  ~step() override
  {

  }

  void run(ossia::token_request t, ossia::exec_state_facade e) noexcept override
  {
    // We want to send a trigger for each value change that happened between last_t and now
    if(t.date > t.prev_date)
    {
      auto& port = *m_outlets[0]->data.target<ossia::value_port>();

      // TODO optimizeme... quite naive for now.
      // TODO maybe start from t.prev_date + 1 ?
      for(int64_t i = t.prev_date.impl; i < t.date.impl; i++)
      {
        if(i % dur == 0)
        {
          port.write_value(values[(i / dur) % values.size()], t.to_tick_time(i));
        }
      }
    }
  }
  ossia::float_vector values;
  ossia::time_value dur{};

  std::string label() const noexcept override
  {
    return "Step";
  }
};


}
