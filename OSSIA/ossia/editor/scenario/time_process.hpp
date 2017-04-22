#pragma once

#include <ossia/editor/scenario/time_value.hpp>
#include <ossia/editor/state/state_element_fwd.hpp>
#include <memory>
#include <ossia_export.h>
#include <string>

namespace ossia
{
class time_constraint;
struct time_value;
/**
 * @brief The time_process class
 *
 * \brief #TimeProcess is the parent class used to describe temporal content.
 */
class OSSIA_EXPORT time_process
{
  friend class time_constraint;
public:
  /*! destructor */
  virtual ~time_process();

  /*! get a #StateElement from the process on its parent #time_constraint offset
   \details don't call offset when the parent #time_constraint is running
   \param const #time_value offset date
   \return state_element */
  virtual state_element offset(ossia::time_value) = 0;

  /*! get a #StateElement from the process depending on its parent
   #time_constraint date
   \details don't call state when the parent #time_constraint is not running
   \return state_element */
  virtual state_element state() = 0;

  /**
  * @brief start
  *
  * Will be called when
  * the parent time constraint is started.
  */
  virtual void start();

  /**
  * @brief stop
  *
  * Will be called when
  * the parent time constraint is stopped.
  */
  virtual void stop();

  /**
  * @brief stop
  *
  * Will be called when
  * the parent time constraint is paused.
  */
  virtual void pause();

  /**
  * @brief resume
  *
  * Will be called when
  * the parent time constraint is resumed.
  */
  virtual void resume();

  /**
   * @brief mute Mutes or unmute the process.
   *
   * `mute(true)` mutes the process.
   * A muted process executes itself but does not send its state.
   */
  void mute(bool m);

  //! True if the process is not currently muted.
  bool unmuted() const;

  /**
   * @brief Enables or disable the process.
   *
   * Unlike a muted process, a disabled process does not execute at all.
   */
  void enable(bool m);

  //! True if the process is enabled.
  bool enabled() const;


  /**
   * @brief parent The parent time_constraint of the process
   *
   * The time_constraint sets this pointer when a process is added to it.
   * This has the obvious consequence that a time_process cannot be part
   * of two different time_constraint.
   *
   * @return A pointer to the parent.
   */
  time_constraint* parent() const;

  protected:
    //! Reimplement this to have a special behaviour on mute
    virtual void mute_impl(bool);
    ossia::time_value m_lastDate{ossia::Infinite};
    // used to filter multiple state calls at the
    // same time (use date as position can be always
    // 0 in infinite duration case)

  private:
    time_constraint* m_parent{};
    bool m_unmuted = true;
    bool m_enabled = true;
};
}
