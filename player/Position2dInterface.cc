/*
 * Copyright 2011 Nate Koenig & Andrew Howard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
/* Desc: Position Interface for Player
 * Author: Nate Koenig
 * Date: 2 March 2006
 */

/**
@addtogroup player
@par Position2d Interface
- PLAYER_POSITION2D_CMD_VEL
- PLAYER_POSITION2D_REQ_SET_ODOM
- PLAYER_POSITION2D_REQ_MOTOR_POWER
*/

/* TODO
- PLAYER_POSITION2D_REQ_GET_GEOM
- PLAYER_POSITION2D_REQ_RESET_ODOM
*/
#include <math.h>
#include <iostream>
#include <boost/thread/recursive_mutex.hpp>

#include "GazeboDriver.hh"
#include "Position2dInterface.hh"

boost::recursive_mutex *Position2dInterface::mutex = NULL;

//////////////////////////////////////////////////
// Constructor
Position2dInterface::Position2dInterface(player_devaddr_t _addr,
    GazeboDriver *_driver, ConfigFile *_cf, int _section)
    : GazeboInterface(_addr, _driver, _cf, _section)
{
  this->datatime = -1;

  if (this->mutex == NULL)
    this->mutex = new boost::recursive_mutex();
}

//////////////////////////////////////////////////
// Destructor
Position2dInterface::~Position2dInterface()
{
}

//////////////////////////////////////////////////
// Handle all messages. This is called from GazeboDriver
int Position2dInterface::ProcessMessage(QueuePointer &_respQueue,
                                        player_msghdr_t *_hdr, void *_data)
{
  int result = 0;

  boost::recursive_mutex::scoped_lock lock(*this->mutex);

  // COMMAND VELOCITY:
  if (Message::MatchMessage(_hdr, PLAYER_MSGTYPE_CMD,
        PLAYER_POSITION2D_CMD_VEL, this->device_addr))
  {
    player_position2d_cmd_vel_t *cmd;

    cmd = (player_position2d_cmd_vel_t*)_data;

    /*this->iface->data->cmdVelocity.pos.x = cmd->vel.px;
    this->iface->data->cmdVelocity.pos.y = cmd->vel.py;
    this->iface->data->cmdVelocity.yaw = cmd->vel.pa;
    */

    result = 0;
  }

  // REQUEST SET ODOMETRY
  else if (Message::MatchMessage(_hdr, PLAYER_MSGTYPE_REQ,
        PLAYER_POSITION2D_REQ_SET_ODOM, this->device_addr))
  {
    if (_hdr->size != sizeof(player_position2d_set_odom_req_t))
    {
      PLAYER_WARN("Arg to odometry set requestes wrong size; ignoring");
      result = -1;
    }
    else
    {
      /*
      player_position2d_set_odom_req_t *odom = 
        (player_position2d_set_odom_req_t*)_data;

      this->iface->data->pose.pos.x = odom->pose.px;
      this->iface->data->pose.pos.y = odom->pose.py;
      this->iface->data->pose.yaw = odom->pose.pa;
      */

      this->driver->Publish(this->device_addr, _respQueue,
          PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_SET_ODOM);

      result = 0;
    }
  }

  // CMD Set Motor Power
  else if (Message::MatchMessage(_hdr, PLAYER_MSGTYPE_CMD,
        PLAYER_POSITION2D_REQ_MOTOR_POWER, this->device_addr))
  {
    // TODO
    result = 0;
  }

  // REQUEST SET MOTOR POWER
  else if (Message::MatchMessage(_hdr, PLAYER_MSGTYPE_REQ,
        PLAYER_POSITION2D_REQ_MOTOR_POWER, this->device_addr))
  {
    if (_hdr->size != sizeof(player_position2d_power_config_t))
    {
      PLAYER_WARN("Arg to motor set requestes wrong size; ignoring");
      result = -1;
    }
    else
    {
      player_position2d_power_config_t *power;

      power = (player_position2d_power_config_t*)_data;

      //this->iface->data->cmdEnableMotors = power->state;

      this->driver->Publish(this->device_addr, _respQueue,
          PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_MOTOR_POWER);

      result = 0;
    }
  }

  // REQUEST GET GEOMETRY
  else if (Message::MatchMessage(_hdr, PLAYER_MSGTYPE_REQ,
        PLAYER_POSITION2D_REQ_GET_GEOM, this->device_addr))
  {
    if (_hdr->size != 0)
    {
      PLAYER_WARN("Arg get robot geom is wrong size; ignoring");
      result = -1;
    }
    else
    {
      player_position2d_geom_t geom;

      // TODO: get correct dimensions; these are for the P2AT
      geom.pose.px = 0;
      geom.pose.py = 0;
      geom.pose.pz = 0;
      geom.pose.pyaw = 0;
      geom.pose.ppitch = 0;
      geom.pose.proll = 0;
      geom.size.sw= 0.53;
      geom.size.sl = 0.38;
      geom.size.sh = 0.31;

      this->driver->Publish(this->device_addr, _respQueue,
          PLAYER_MSGTYPE_RESP_ACK,
          PLAYER_POSITION2D_REQ_GET_GEOM,
          &geom, sizeof(geom), NULL);

      result = 0;
    }
  }

  // REQUEST RESET ODOMETRY
  else if (Message::MatchMessage(_hdr, PLAYER_MSGTYPE_REQ,
           PLAYER_POSITION2D_REQ_RESET_ODOM, this->device_addr))
  {
    if (_hdr->size != 0)
    {
      PLAYER_WARN("Arg reset position request is wrong size; ignoring");
      result = -1;
    }
    else
    {
      // TODO: Make this work!!
      this->driver->Publish(this->device_addr, _respQueue,
          PLAYER_MSGTYPE_RESP_ACK, PLAYER_POSITION2D_REQ_RESET_ODOM);
      result = 0;
    }
  }
  else
  {
    result = -1;
  }

  return result;
}

//////////////////////////////////////////////////
// Update this interface, publish new info. This is
// called from GazeboDriver::Update
void Position2dInterface::Update()
{
  /*
  player_position2d_data_t data;
  struct timeval ts;

  memset(&data, 0, sizeof(data));

  boost::recursive_mutex::scoped_lock lock(*this->mutex);
  // Only Update when new data is present
  if (this->iface->data->head.time > this->datatime)
  {
    this->datatime = this->iface->data->head.time;

    ts.tv_sec = (int) (this->iface->data->head.time);
    ts.tv_usec = (int) (fmod(this->iface->data->head.time, 1) * 1e6);

    data.pos.px = this->iface->data->pose.pos.x;
    data.pos.py = this->iface->data->pose.pos.y;
    data.pos.pa = this->iface->data->pose.yaw;

    data.vel.px = this->iface->data->velocity.pos.x;
    data.vel.py = this->iface->data->velocity.pos.y;
    data.vel.pa = this->iface->data->velocity.yaw;

    data.stall = (uint8_t) this->iface->data->stall;

    this->driver->Publish( this->device_addr,
        PLAYER_MSGTYPE_DATA,
        PLAYER_POSITION2D_DATA_STATE,
        (void*)&data, sizeof(data), &this->datatime );

  }
  */
}


//////////////////////////////////////////////////
// Open a SHM interface when a subscription is received. This is called from
// GazeboDriver::Subscribe
void Position2dInterface::Subscribe()
{
}

//////////////////////////////////////////////////
// Close a SHM interface. This is called from GazeboDriver::Unsubscribe
void Position2dInterface::Unsubscribe()
{
}
