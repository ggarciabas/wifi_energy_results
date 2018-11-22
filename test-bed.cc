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
#include "test-bed.h"
#include "ns3/log.h"
#include "uav-energy-source-helper.h"
#include "uav-energy-source.h"
#include "ns3/energy-module.h"
#include "ns3/simulator.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/dsr-module.h"
#include <fstream>
#include <cstdlib>
#include <string>

// #define LOG_FILE

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TestBed");

NS_OBJECT_ENSURE_REGISTERED(TestBed);

TypeId
TestBed::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::TestBed")
                          .SetParent<Object>()
                          .SetGroupName("Wifi-Energy-Results")
                          .AddConstructor<TestBed>()
                          .AddAttribute("SimulationTime", "Tempo de simulacao",
                                        DoubleValue(100),
                                        MakeDoubleAccessor(&TestBed::m_simTime),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("Scenario",
                                        "Minimum x of the scenario",
                                        UintegerValue(1),
                                        MakeUintegerAccessor(&TestBed::m_scenario),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("Protocol",
                                        "Protocol.",
                                        UintegerValue(1),
                                        MakeUintegerAccessor(&TestBed::m_protocol),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("Seed",
                                        "Seed.",
                                        UintegerValue(1),
                                        MakeUintegerAccessor(&TestBed::m_seed),
                                        MakeUintegerChecker<uint32_t>())
                          ;
  return tid;
}

TestBed::TestBed()
{
  NS_LOG_FUNCTION(this);
  m_meanConsumptionAdhoc = 0.0;
  m_meanConsumption = 0.0;
  m_countTx = 0;
  m_countRx = 0;
  m_consAdhoc = 0;
  m_consWifi = 0;
  m_initialEnergyJ = 500.0;
}

TestBed::~TestBed()
{
  NS_LOG_FUNCTION(this);
}

void TestBed::Run () {
  switch (m_scenario)
  {
    case 1:
      m_scenarioName = "teste_1";
      break;
    case 2:
      m_scenarioName = "teste_2";
      break;
    case 3:
      m_scenarioName = "teste_3";
      break;
    default:
      NS_FATAL_ERROR("Não foi possivel identificar o cenario!");
  }
  switch (m_protocol)
  {
  case 1:
    m_protocolName = "OLSR";
    break;
  case 2:
    m_protocolName = "AODV";
    break;
  case 3:
    m_protocolName = "DSDV";
    break;
  default:
    NS_FATAL_ERROR("No such protocol:" << m_protocol);
  }
  std::ostringstream ss;
  ss << "rm -Rf ./scratch/wifi_energy_results/data/output/" << m_scenarioName << "/" << m_protocolName << "/" << m_seed;
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/wifi_energy_results/data/output/" << m_scenarioName << "/" << m_protocolName << "/" << m_seed;
  system(ss.str().c_str());

  ConfigureScenario();

  m_eventAdhoc = Simulator::Schedule(Seconds(5.0), &TestBed::PrintConsumptionAdhoc, this);
  m_eventWifi = Simulator::Schedule(Seconds(5.0), &TestBed::PrintConsumptionWifi, this);

  Simulator::Stop(Seconds(m_simTime));
  NS_LOG_DEBUG("Iniciando Simulador @"<<Simulator::Now().GetSeconds());
  Simulator::Run();
  NS_LOG_DEBUG("Finalizando Simulador @"<<Simulator::Now().GetSeconds());
  Simulator::Destroy();
  NS_LOG_DEBUG("Finalizando Destroy @"<<Simulator::Now().GetSeconds());

  std::ostringstream os;
  os << "./scratch/wifi_energy_results/data/output/" << m_scenarioName << "/" << m_protocolName << "/" << m_seed << "/final.txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << m_countTx << "," << m_countRx << "\n" << m_meanConsumption << "," << m_meanConsumptionAdhoc << "\n";
  file.close();

  os.str("");
  os << "./scratch/wifi_energy_results/data/output/" << m_scenarioName << "/" << m_protocolName << "/" << m_seed << "/scenario.txt";
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  Vector serv = m_server.Get(0)->GetObject<MobilityModel>()->GetPosition();
  file << serv.x << "," << serv.y << "\n";
  Vector avaliado = m_uavs.Get(0)->GetObject<MobilityModel>()->GetPosition();
  file << avaliado.x << "," << avaliado.y << "\n";
  bool f = true;
  for (NodeContainer::Iterator i = m_uavs.Begin() ; i != m_uavs.End(); ++i)
  {
    Vector current = (*i)->GetObject<MobilityModel>()->GetPosition();
    if (f) {
      file << current.x << "," << current.y;
      f = false;
    } else {
      file << "," << current.x << "," << current.y;
    }
  }
  file << "\n";
  f = true;
  for (NodeContainer::Iterator i = m_cli.Begin() ; i != m_cli.End(); ++i)
  {
    Vector current = (*i)->GetObject<MobilityModel>()->GetPosition();
    if (f) {
      file << current.x << "," << current.y;
      f = false;
    } else {
      file << "," << current.x << "," << current.y;
    }
  }
  file.close();

}

void TestBed::ConfigureScenario ()
{
  // lendo arquivo
  std::ostringstream ss;
  std::ifstream scenario;
  ss << "./scratch/wifi_energy_results/data/scenarios/" << m_scenarioName << ".txt";
  scenario.open(ss.str().c_str());
  int qtUav;
  double x, y;
  if (scenario.is_open())
  {
    std::string line;
    getline(scenario, line);
    sscanf(line.c_str(), "%d\n", &qtUav);
    getline(scenario, line);
    sscanf(line.c_str(), "%lf,%lf\n", &x, &y);
    scenario.close();
  } else {
    NS_FATAL_ERROR("Problema no arquivo, abertura.");
  }

  NS_LOG_DEBUG ("(" << x << "," << y << ") - " << qtUav << " @"<<Simulator::Now().GetSeconds());

  // configurando variaveis
  // Routing
  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  Ipv4StaticRoutingHelper staticR;
  Ipv4ListRoutingHelper list;
  list.Add(staticR, 100);
  switch (m_protocol)
  {
  case 1:
    list.Add(olsr, 100);
    break;
  case 2:
    list.Add(aodv, 100);
    break;
  case 3:
    list.Add(dsdv, 100);
    break;
  default:
    NS_FATAL_ERROR("No such protocol:" << m_protocol);
  }

  // configurando INTERNET
  InternetStackHelper stack;
  stack.SetRoutingHelper(list);

  // Ad Hoc
  WifiHelper adhocHelper;
  adhocHelper.SetStandard(WIFI_PHY_STANDARD_80211ac); // http://www.mwnl.snu.ac.kr/~schoi/publication/Conferences/15-SECON-LEE.pdf
  adhocHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("VhtMcs1"),
                                "ControlMode", StringValue ("VhtMcs0"));
  YansWifiPhyHelper phyHelper;
  phyHelper = YansWifiPhyHelper::Default();
  phyHelper.Set ("TxPowerStart", DoubleValue (17)); // http://www.mwnl.snu.ac.kr/~schoi/publication/Conferences/15-SECON-LEE.pdf
  phyHelper.Set ("TxPowerEnd", DoubleValue (17));
  phyHelper.Set ("TxPowerLevels", UintegerValue (1));

  YansWifiChannelHelper channelHelper;
  channelHelper = YansWifiChannelHelper::Default();
  channelHelper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  double rss = -80;  // -dBm
  channelHelper.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  phyHelper.SetChannel(channelHelper.Create());
  // Set it to adhoc mode
  WifiMacHelper macAdHocHelper;
  macAdHocHelper.SetType("ns3::AdhocWifiMac");
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.1.1.0", "255.255.255.0"); // adhoc address

  // Wifi
  YansWifiPhyHelper phyHelperCli;
  phyHelperCli = YansWifiPhyHelper::Default();
  phyHelper.Set ("TxPowerStart", DoubleValue (10));
  phyHelper.Set ("TxPowerEnd", DoubleValue (10));
  phyHelper.Set ("TxPowerLevels", UintegerValue (1));
  YansWifiChannelHelper channelHelperCli;
  channelHelperCli = YansWifiChannelHelper::Default();
  phyHelperCli.SetChannel(channelHelperCli.Create());
  WifiHelper wifiHelper;
  wifiHelper.SetStandard(WIFI_PHY_STANDARD_80211n_2_4GHZ); // https://dl.acm.org/citation.cfm?id=1924928 (http://www.inf.ed.ac.uk/teaching/courses/cn/papers/halperin-hotpower10.pdf)
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                          "DataMode", StringValue ("HtMcs1"),
                                          "ControlMode", StringValue ("HtMcs0"));
  WifiMacHelper macWifiHelperCli;
  macWifiHelperCli.SetType("ns3::StaWifiMac",
                             "Ssid", SsidValue(Ssid("flynetwork")),
                             "ActiveProbing", BooleanValue(false)); // configuração de scanning passivo
  WifiMacHelper macWifiHelper;
  macWifiHelper.SetType("ns3::ApWifiMac",
                          "Ssid", SsidValue(Ssid("flynetwork")));
  Ipv4AddressHelper addressHelperCli;
  addressHelperCli.SetBase("192.168.1.0", "255.255.255.0"); // wifi address

  // ----- configure server
  // criando no
  m_server.Create(1);
  // criando device
  NetDeviceContainer adhoc = adhocHelper.Install(phyHelper, macAdHocHelper, m_server);
  // configurando internet
  stack.Install(m_server);
  // configurando pilha de protocolos
  Ipv4InterfaceContainer serverAddress;
  serverAddress.Add(addressHelper.Assign(adhoc));
  // posicionar servidor no centro
  Ptr<ConstantPositionMobilityModel> serverMobility = CreateObject<ConstantPositionMobilityModel>();
  serverMobility->SetPosition(Vector(x/2.0, y/2.0, 5.0));
  m_server.Get(0)->AggregateObject(serverMobility);
  // configurando PacketSink
  ObjectFactory packVoice;
  packVoice.SetTypeId ("ns3::PacketSink");
  packVoice.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
  packVoice.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), 5060))); // OPS: utilizam Ipv4Address::GetAny ()
  Ptr<Application> appVoice = packVoice.Create<Application> ();
  appVoice->SetStartTime(Seconds(10.0));
  appVoice->SetStopTime(Seconds(m_simTime));
  m_server.Get(0)->AddApplication (appVoice);
  appVoice->TraceConnectWithoutContext ("Rx", MakeCallback (&TestBed::TracedCallbackRxAppServer, this));

  ObjectFactory packVideo;
  packVideo.SetTypeId ("ns3::PacketSink");
  packVideo.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
  packVideo.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), 5070))); // OPS: utilizam Ipv4Address::GetAny ()
  Ptr<Application> appVideo = packVideo.Create<Application> ();
  appVideo->SetStartTime(Seconds(10.0));
  appVideo->SetStopTime(Seconds(m_simTime));
  m_server.Get(0)->AddApplication (appVideo);
  appVideo->TraceConnectWithoutContext ("Rx", MakeCallback (&TestBed::TracedCallbackRxAppServer, this));

  ObjectFactory packWww;
  packWww.SetTypeId ("ns3::PacketSink");
  packWww.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
  packWww.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), 8080))); // OPS: utilizam Ipv4Address::GetAny ()
  Ptr<Application> appWww = packWww.Create<Application> ();
  appWww->SetStartTime(Seconds(10.0));
  appWww->SetStopTime(Seconds(m_simTime));
  m_server.Get(0)->AddApplication (appWww);
  appWww->TraceConnectWithoutContext ("Rx", MakeCallback (&TestBed::TracedCallbackRxAppServer, this));

  // configure UAV
  m_uavs.Create(qtUav);
  // criando device Adhoc
  NetDeviceContainer adhocUav = adhocHelper.Install(phyHelper, macAdHocHelper, m_uavs);
  // configura comunicação wifi
  NetDeviceContainer wifi = wifiHelper.Install(phyHelperCli, macWifiHelper, m_uavs);
  // configurando internet
  stack.Install(m_uavs);
  // configurando pilha de protocolos
  addressHelper.Assign(adhocUav);
  Ipv4InterfaceContainer wifiAddr = addressHelperCli.Assign(wifi);

  // mobilidade
  Ptr< PositionAllocator >    positionAlloc;
  ss.str("");
  ss << "ns3::UniformRandomVariable[Min="<< x/5<<"|Max=" << x/5*4 << "]";
  positionAlloc = CreateObjectWithAttributes<RandomDiscPositionAllocator>
                      ("X", DoubleValue (x/2.0),
                        "Y", DoubleValue (y/2.0),
                        "Rho", StringValue(ss.str().c_str()));

  for (NodeContainer::Iterator i = m_uavs.Begin(); i != m_uavs.End(); ++i) {
    Vector v = positionAlloc->GetNext();
    v.z = 10.0;
    Ptr<ConstantPositionMobilityModel> mob = CreateObject<ConstantPositionMobilityModel>();
    mob->SetPosition(v);
    (*i)->AggregateObject(mob);
  }

  DhcpHelper dhcpHelper;
  // Configure DHCP
  int c = 0;
  for (NodeContainer::Iterator i = m_uavs.Begin(); i != m_uavs.End(); ++i, ++c) {
    // The router must have a fixed IP.
    std::ostringstream oss, poolAddr, minAddr, maxAddr, serverAddr;
    poolAddr.str(""); minAddr.str(""); maxAddr.str(""); serverAddr.str("");
    serverAddr << "192.168." << (*i)->GetId() << ".1";
    minAddr << "192.168." << (*i)->GetId() << ".2";
    maxAddr << "192.168." << (*i)->GetId() << ".254";
    poolAddr << "192.168." << (*i)->GetId() << ".0";
    Ipv4InterfaceContainer fixedNodes = dhcpHelper.InstallFixedAddress (wifi.Get (c), Ipv4Address (serverAddr.str().c_str()), Ipv4Mask ("/24"));
    // Not really necessary, IP forwarding is enabled by default in IPv4.
    fixedNodes.Get (0).first->SetAttribute ("IpForward", BooleanValue (true));
    // DHCP server
    ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer (wifi.Get (c),
                        Ipv4Address (serverAddr.str().c_str()),
                        Ipv4Address (poolAddr.str().c_str()), Ipv4Mask ("/24"),
                        Ipv4Address (minAddr.str().c_str()), Ipv4Address (maxAddr.str().c_str()),
                        Ipv4Address (serverAddr.str().c_str()));
    dhcpServerApp.Start (Seconds (10.0));
    dhcpServerApp.Stop (Seconds(m_simTime));
  }

  // energia somente em um UAV
  /* Mine energy source */
  // UavEnergySourceHelper sourceHelper;
  // sourceHelper.Set("ScenarioName", StringValue(m_scenarioName));
  // sourceHelper.Set("UavEnergySourceInitialEnergy", DoubleValue(m_initialEnergyJ)); // Joules
  // sources = sourceHelper.Install(m_uavs.Get(0)); // install source
  //// DynamicCast<UavEnergySource>(sources.Get(0))->Start(); --- not used in this simulation

  // Energy sources
  EnergySourceContainer sources;
  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (m_initialEnergyJ));
  basicSourceHelper.Set ("BasicEnergySupplyVoltageV", DoubleValue (3.3));
  sources = basicSourceHelper.Install(m_uavs.Get(0));

  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // caculator : https://www.rapidtables.com/calc/electric/Watt_to_Amp_Calculator.html
  // OBs. utilizando 3.3V!
  // ---- Wifi: We  place  a current sense resistor (40 mΩ) on the 3.3 V power supply to the NIC. [http://www.inf.ed.ac.uk/teaching/courses/cn/papers/halperin-hotpower10.pdf]
  // ---- Adhoc: We  put  a 40 mΩ resistor in front of the power pin of the adapter which supplies  3.3  V  to  the  NIC. [http://www.mwnl.snu.ac.kr/~schoi/publication/Conferences/15-SECON-LEE.pdf]
  // configure radio energy model

  //  WIFI https://dl.acm.org/citation.cfm?id=1924928 (http://www.inf.ed.ac.uk/teaching/courses/cn/papers/halperin-hotpower10.pdf)
  radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (0.24848484848)); // Ampere - Idle 0.82W
  radioEnergyHelper.Set ("CcaBusyCurrentA", DoubleValue (0.24848484848)); // Ampere - same as idle
  radioEnergyHelper.Set ("SwitchingCurrentA", DoubleValue (0.24848484848)); // Ampere - same as idle
  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.030303030303)); // Ampere - Sleep 0.1W
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.38787878788)); // Ampere - Tx 1.28W
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.28484848485)); // Ampere - Rx 0.94W
  radioEnergyHelper.SetDepletionCallback (MakeCallback(&TestBed::WifiRadioEnergyDepletionCallback, this));
  DeviceEnergyModelContainer deviceModelsWifi = radioEnergyHelper.Install (wifi.Get(0), sources);

  // ADHOC -- http://www.mwnl.snu.ac.kr/~schoi/publication/Conferences/15-SECON-LEE.pdf [1 antena]
  radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (0.18181818182)); // Ampere - (17dBm 40MHz) Idle 0.6W
  radioEnergyHelper.Set ("CcaBusyCurrentA", DoubleValue (0.18181818182)); // Ampere - same as idle
  radioEnergyHelper.Set ("SwitchingCurrentA", DoubleValue (0.18181818182)); // Ampere - same as idle
  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.030303030303)); // Ampere - Sleep 0.1W
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.45454545455)); // Ampere - Tx 1.5W
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.25757575758)); // Ampere - Rx 0.85W
  // install device model
  DeviceEnergyModelContainer deviceModelsAdhoc = radioEnergyHelper.Install (adhocUav.Get(0), sources);

  // Configure TotalEnergyConsumption
  deviceModelsAdhoc.Get(0)->TraceConnectWithoutContext ("TotalEnergyConsumption", MakeCallback(&TestBed::TotalEnergyConsumptionTraceAdhoc,  this));
  deviceModelsWifi.Get(0)->TraceConnectWithoutContext ("TotalEnergyConsumption", MakeCallback(&TestBed::TotalEnergyConsumptionTrace,  this));

  // configure cli
  m_cli.Create(5);
  // configurando devices
  NetDeviceContainer dev  = wifiHelper.Install(phyHelperCli, macWifiHelperCli, m_cli);
  // configurando internet
  InternetStackHelper stackCli;
  stackCli.Install(m_cli);
  // mobilidade
  MobilityHelper mobilityCLI;
  // ss.str("");
  // ss << "ns3::UniformRandomVariable[Min=0|Max=" << x/5 << "]";
  NS_LOG_DEBUG (m_uavs.Get(0)->GetObject<MobilityModel>()->GetPosition().x << "," << m_uavs.Get(0)->GetObject<MobilityModel>()->GetPosition().y);
  Ptr<PositionAllocator> positionAllocCli = CreateObjectWithAttributes<RandomDiscPositionAllocator>
                      ("X", DoubleValue (m_uavs.Get(0)->GetObject<MobilityModel>()->GetPosition().x),
                        "Y", DoubleValue (m_uavs.Get(0)->GetObject<MobilityModel>()->GetPosition().y),
                      "Rho", StringValue("ns3::UniformRandomVariable[Min=2|Max=30]"));
  mobilityCLI.SetPositionAllocator(positionAllocCli);
  // NS_LOG_DEBUG (x/5 << " " << x/5*4 << " "  << y/5 << " " << y/5*4);
  // mobilityCLI.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
  //                              "Bounds", RectangleValue(Rectangle(x/5, x/5*4, y/5, y/5*4)),
  //                               "Speed", StringValue("ns3::ConstantRandomVariable[Constant=3.0]"));
  mobilityCLI.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityCLI.Install(m_cli);
  // dhcp
  ApplicationContainer dhcpClients = dhcpHelper.InstallDhcpClient (dev);
  dhcpClients.Start (Seconds (10.0));
  dhcpClients.Stop (Seconds(m_simTime));
  // aplicacoes
  Ptr<UniformRandomVariable> app_rand = CreateObject<UniformRandomVariable>(); // Padrão [0,1]
  app_rand->SetAttribute ("Min", DoubleValue (0));
  app_rand->SetAttribute ("Max", DoubleValue (4));

  for (NodeContainer::Iterator i = m_cli.Begin(); i!=m_cli.End(); ++i) {
    // configure OnOff application para server
    int app_code = app_rand->GetValue();
    int port = 0;
    ObjectFactory onoffFac;
    Ptr<Application> appOnOff = 0;
    if (app_code < 1) { // VOICE
        onoffFac.SetTypeId ("ns3::OnOffApplication");
        onoffFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
        onoffFac.Set ("PacketSize", UintegerValue (50));
        onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
        onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        // P.S.: offTime + DataRate/PacketSize = next packet time
        onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.024Mbps")));
        port = 5060;
        onoffFac.Set ("Remote", AddressValue (InetSocketAddress (serverAddress.GetAddress(0), port)));
        appOnOff = onoffFac.Create<Application> ();
        appOnOff->SetStartTime(Seconds(10.0));
        appOnOff->SetStopTime(Seconds(m_simTime));
        (*i)->AddApplication (appOnOff);
        appOnOff->TraceConnectWithoutContext ("Tx", MakeCallback (&TestBed::TracedCallbackTx, this));
    } else if (app_code < 2) { // VIDEO
        onoffFac.SetTypeId ("ns3::OnOffApplication");
        onoffFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
        onoffFac.Set ("PacketSize", UintegerValue (429));
        onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
        onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        // P.S.: offTime + DataRate/PacketSize = next packet time
        onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.128Mbps")));
        port = 5070;
        onoffFac.Set ("Remote", AddressValue (InetSocketAddress (serverAddress.GetAddress(0), port)));
        appOnOff = onoffFac.Create<Application> ();
        appOnOff->SetStartTime(Seconds(10.0));
        appOnOff->SetStopTime(Seconds(m_simTime));
        (*i)->AddApplication (appOnOff);
        appOnOff->TraceConnectWithoutContext ("Tx", MakeCallback (&TestBed::TracedCallbackTx, this));
    } else if (app_code < 3) { // WWW
        onoffFac.SetTypeId ("ns3::OnOffApplication");
        onoffFac.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
        onoffFac.Set ("PacketSize", UintegerValue (429));
        onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
        onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.04]"));
        // P.S.: offTime + DataRate/PacketSize = next packet time
        onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.128Mbps")));
        port = 8080;
        onoffFac.Set ("Remote", AddressValue (InetSocketAddress (serverAddress.GetAddress(0), port)));
        appOnOff = onoffFac.Create<Application> ();
        appOnOff->SetStartTime(Seconds(10.0));
        appOnOff->SetStopTime(Seconds(m_simTime));
        (*i)->AddApplication (appOnOff);
        appOnOff->TraceConnectWithoutContext ("Tx", MakeCallback (&TestBed::TracedCallbackTx, this));
    } else if (app_code < 4) { // NOTHING
    } else NS_FATAL_ERROR ("UavNetwork .. application error");

  }

}

void
TestBed::TracedCallbackRxAppServer (Ptr<const Packet> packet, const Address & address)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("Rx Server  @"<<Simulator::Now().GetSeconds());
  m_countRx++;
}

void
TestBed::PrintConsumptionAdhoc ()
{
  std::ostringstream os;
  os << "./scratch/wifi_energy_results/data/output/" << m_scenarioName << "/" << m_protocolName << "/" << m_seed << "/consumption_cresc_adhoc.txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << "," << m_consCrescAdhoc << std::endl;
  file.close();

  os.str("");
  os << "./scratch/wifi_energy_results/data/output/" << m_scenarioName << "/" << m_protocolName << "/" << m_seed << "/consumption_atual_adhoc.txt";
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << "," << m_consAdhoc << std::endl;
  file.close();

  m_consAdhoc = 0;
  m_eventAdhoc = Simulator::Schedule(Seconds(5.0), &TestBed::PrintConsumptionAdhoc, this);
}

void TestBed::PrintConsumptionWifi ()
{
  std::ostringstream os;
  os << "./scratch/wifi_energy_results/data/output/" << m_scenarioName << "/" << m_protocolName << "/" << m_seed << "/consumption_cresc_wifi.txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << "," << m_consCrescWifi << std::endl;
  file.close();

  os.str("");
  os << "./scratch/wifi_energy_results/data/output/" << m_scenarioName << "/" << m_protocolName << "/" << m_seed << "/consumption_atual_wifi.txt";
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << "," << m_consWifi << std::endl;
  file.close();

  m_consWifi = 0;
  m_eventWifi = Simulator::Schedule(Seconds(5.0), &TestBed::PrintConsumptionWifi, this);
}

void
TestBed::TotalEnergyConsumptionTrace (double oldV, double newV)
{
  NS_LOG_FUNCTION(this);
  // GetInitialEnergy
  m_meanConsumption += (newV - oldV);
  m_meanConsumption /= 2.0;

  m_consWifi += (newV - oldV);
  m_consCrescWifi = newV;
}

void
TestBed::TotalEnergyConsumptionTraceAdhoc (double oldV, double newV)
{
  NS_LOG_FUNCTION(this);
  m_meanConsumptionAdhoc += (newV - oldV);
  m_meanConsumptionAdhoc /= 2.0;

  m_consAdhoc += (newV - oldV);
  m_consCrescAdhoc = newV;
}

void
TestBed::TracedCallbackTx (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("Tx Server  @"<<Simulator::Now().GetSeconds());
  m_countTx++;
}

void
TestBed::WifiRadioEnergyDepletionCallback ()
{
  NS_LOG_DEBUG("Energy Depletion  @" << Simulator::Now().GetSeconds());
  m_eventAdhoc.Cancel();
  m_eventWifi.Cancel();
  PrintConsumptionAdhoc();
  PrintConsumptionWifi();
  m_eventAdhoc.Cancel();
  m_eventWifi.Cancel();
  Simulator::Stop();
}

}
