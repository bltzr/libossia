#pragma once
#include <ossia/audio/audio_protocol.hpp>
namespace ossia
{
class dummy_engine final
    : public audio_engine
{
  int m_rate{}, m_bs{};
  public:
    dummy_engine(std::string_view name, int& inputs, int& outputs, int rate, int bs)
    {
      //inputs = 0;
      //outputs = 0;
      m_rate = rate;
      m_bs = bs;

      setup_thread();
    }

    void setup_thread()
    {
      int us_per_buffer = 1e6 * double(m_bs) / double(m_rate);
      m_runThread = std::thread{[=] {
        while(!stop_processing)
        {
          if(auto proto = protocol.load())
          {
            proto->process_generic(*proto, nullptr, nullptr, 0, 0, m_bs);
            proto->audio_tick(m_bs, 0);

            // Run a tick
            if(proto->replace_tick)
            {
              proto->audio_tick = std::move(proto->ui_tick);
              proto->ui_tick = {};
              proto->replace_tick = false;
            }
          }
          std::this_thread::sleep_for(std::chrono::microseconds(us_per_buffer));
        }
      }};
    }

    ~dummy_engine() override
    {

    }

    void reload(audio_protocol* p) override
    {
      if(this->protocol)
        this->protocol.load()->engine = nullptr;
      stop();

      this->protocol = p;
      if(!p)
        return;

      auto& proto = *p;
      proto.engine = this;

      proto.setup_tree(0, 0);

      stop_processing = false;
      setup_thread();
    }

    void stop() override
    {
      stop_processing = true;
      protocol = nullptr;

      while(processing) std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if(m_runThread.joinable())
        m_runThread.join();
    }

  private:
    std::thread m_runThread;
};

}
