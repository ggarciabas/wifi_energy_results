/* Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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

using namespace ns3;

int main (int argc, char *argv[])
{
	double sim_time=200.0;
	uint32_t scenario = 1, protocol = 2, seed=6112018;
	CommandLine cmd;
  bool debug=false;
  cmd.AddValue ("Seed", "Seed", seed);
  cmd.AddValue ("Debug", "Debug", debug);
  cmd.AddValue ("Scenario", "Scenario", scenario);
  cmd.AddValue ("Protocol", "Routing Protocol: 1-OLSR 2-AODV 3-DSDV", protocol);
  cmd.Parse (argc, argv);

	SeedManager::SetSeed(seed);

  if (debug) {
    // LogComponentEnable("TestBed", LOG_FUNCTION);
		LogComponentEnable("TestBed", LOG_DEBUG);
		// LogComponentEnable("DhcpHelper", LOG_ALL);
  }
  ObjectFactory obj;
	obj.SetTypeId("ns3::TestBed");
	obj.Set("SimulationTime", DoubleValue(sim_time));
	obj.Set("Scenario", UintegerValue(scenario));
	obj.Set("Protocol", UintegerValue(protocol));
	obj.Set("Seed", UintegerValue(seed));
	Ptr<TestBed> tb = obj.Create()->GetObject<TestBed>();
	tb->Run();
	tb->Dispose();
}
