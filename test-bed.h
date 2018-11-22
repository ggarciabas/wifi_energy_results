/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Giovanna Garcia <ggarciabas@gmail.com>
 */

#ifndef ADHOC_ENERGY
#define ADHOC_ENERGY

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/stats-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/olsr-module.h"

namespace ns3
{

/**
 * TestBed
 */
class TestBed : public Object
{
public:
  static TypeId GetTypeId(void);
  TestBed();
  ~TestBed();
  void Run ();
  void TracedCallbackRxAppServer (Ptr<const Packet> packet, const Address & address);
  void TracedCallbackTx (Ptr<const Packet> packet);
  void TotalEnergyConsumptionTraceAdhoc (double oldV, double newV);
  void TotalEnergyConsumptionTrace (double oldV, double newV);

  void WifiRadioEnergyDepletionCallback ();

  void PrintConsumptionAdhoc ();
  void PrintConsumptionWifi ();

private:
  void ConfigureScenario();

private:
  double m_simTime;
  uint32_t m_scenario;
  uint32_t m_seed;
  uint32_t m_protocol;
  std::string m_scenarioName;
  double m_initialEnergyJ;

  double m_meanConsumptionAdhoc;
  double m_meanConsumption;

  double m_consAdhoc, m_consCrescAdhoc;
  double m_consWifi, m_consCrescWifi;

  EventId m_eventAdhoc;
  EventId m_eventWifi;

  // ns3
  NodeContainer m_uavs;
  NodeContainer m_server;
  NodeContainer m_cli;

  int m_countTx;
  int m_countRx;

  std::string m_protocolName;

};

}

#endif // ADHOC_ENERGY
