#include <ossia/editor/scenario/time_process.hpp>
#include <ossia/editor/state/state_element.hpp>

namespace ossia
{
time_process::~time_process()
{
}

state_element time_process::offset(time_value date, double pos)
{
  return state(date, pos);
}

void time_process::start()
{
}

void time_process::stop()
{
}

void time_process::pause()
{
}

void time_process::resume()
{
}

void time_process::mute(bool m)
{
  m_unmuted = !m;
  mute_impl(m);
}

bool time_process::unmuted() const
{
  return m_unmuted;
}

void time_process::enable(bool m)
{
  m_enabled = m;
}

bool time_process::enabled() const
{
  return m_enabled;
}

time_constraint*time_process::parent() const
{
  return m_parent;
}

void time_process::mute_impl(bool)
{

}
}
