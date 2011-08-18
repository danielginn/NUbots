/*! @file DarwinSensors.cpp
    @brief Implementation of Darwin sensor class

    @author Jason Kulk
 
  Copyright (c) 2010 Jason Kulk
 
 This file is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This file is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with NUbot.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DarwinSensors.h"
#include "Infrastructure/NUSensorsData/NUSensorsData.h"



#include "Tools/Math/General.h"

#include "debug.h"
#include "debugverbositynusensors.h"

#include <limits>

using namespace std;

/*! @brief Constructs a nubot sensor class with Bear backend
 */
DarwinSensors::DarwinSensors(DarwinPlatform* darwin, Robot::CM730* subboard)
{
    #if DEBUG_NUSENSORS_VERBOSITY > 0
        debug << "DarwinSensors::DarwinSensors()" << endl;
    #endif

	platform = darwin;
	cm730 = subboard;

    m_data->addSensors(platform->m_servo_names);

	m_joint_ids = m_data->mapIdToIds(NUSensorsData::All);
    m_previous_positions = vector<float>(platform->m_servo_names.size(), 0);
    m_previous_velocities = vector<float>(platform->m_servo_names.size(), 0);

	
}

/*! @brief Destructor for DarwinSensors
 */
DarwinSensors::~DarwinSensors()
{
    #if DEBUG_NUSENSORS_VERBOSITY > 0
        debug << "DarwinSensors::~DarwinSensors()" << endl;
    #endif
	delete cm730;
}

/*! @brief Copys the sensors data from the hardware communication module to the NUSensorsData container
 */
void DarwinSensors::copyFromHardwareCommunications()
{
    copyFromJoints();
    //copyFromAccelerometerAndGyro();
    copyFromFeet();
    copyFromButtons();
    copyFromBattery();
}

/*! @brief Copys the joint sensor data 
 */
void DarwinSensors::copyFromJoints()
{
	
	static const float NaN = numeric_limits<float>::quiet_NaN();
	vector<float> joint(NUSensorsData::NumJointSensorIndices, NaN);
    float delta_t = (m_current_time - m_previous_time)/1000;
	int data;

	int start_addr = 0;//int(Robot::MX28::P_TORQUE_ENABLE);
	int end_addr   = int(Robot::MX28::P_PRESENT_TEMPERATURE);
	int datasize   = end_addr-start_addr+1;
	unsigned char* datatable = new unsigned char[datasize];
	int addr;
	int error;

	for (size_t i=0; i < platform->m_servo_IDs.size(); i++)
    {

		cm730->ReadTable(int(platform->m_servo_IDs[i]),start_addr,end_addr,datatable,&error);

		/*cout << int(platform->m_servo_IDs[i]) << ": ";
		for(int j = 0; j < datasize; j++)
		{
			cout << int(datatable[j]) << " ";
		}
		cout << endl;*/
		/*if(error != 0)
		{
			cout << Platform->getTime() << "\t Motor error: \t" << platform->m_servo_names[i] << endl;
		}*/

		//cm730->ReadWord(int(platform->m_servo_IDs[i]),int(Robot::MX28::P_PRESENT_POSITION_L), &(data), 0); 	//<! Read Position
		addr = int(Robot::MX28::P_PRESENT_POSITION_L);
		data = cm730->MakeWord(datatable[addr-start_addr],datatable[addr-start_addr+1]);
		joint[NUSensorsData::PositionId] = Value2Radian(data) + platform->m_servo_Offsets[i];
		//cout << addr-start_addr << " " << int(datatable[addr-start_addr]) << endl;
		//cout << data << "\t" <<newdata << endl;
		
		//cm730->ReadWord(int(platform->m_servo_IDs[i]),int(Robot::MX28::P_GOAL_POSITION_L), &(data), 0); 			//<! Read Goal Position (target)
		addr = int(Robot::MX28::P_GOAL_POSITION_L);
		data = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);
		joint[NUSensorsData::TargetId] = Value2Radian(data) + platform->m_servo_Offsets[i];
		
		//cm730->ReadWord(int(platform->m_servo_IDs[i]),int(Robot::MX28::P_MOVING_SPEED_L), &(data), 0); 		//<! Read Velocity
		addr = int(Robot::MX28::P_MOVING_SPEED_L);
		data = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);
		joint[NUSensorsData::VelocityId] = data;

		//cm730->ReadByte(int(platform->m_servo_IDs[i]),int(Robot::MX28::P_PRESENT_TEMPERATURE), &(data), 0); //<! Read Temperature
		addr = int(Robot::MX28::P_PRESENT_TEMPERATURE);
		data = int(datatable[addr-start_addr]);
		joint[NUSensorsData::TemperatureId] = data;

		//cm730->ReadByte(int(platform->m_servo_IDs[i]),int(Robot::MX28::P_TORQUE_ENABLE), &(data), 0); 		//<! Read Stiffness
		addr = int(Robot::MX28::P_TORQUE_ENABLE);
		data = int(datatable[addr-start_addr]);
		joint[NUSensorsData::StiffnessId] = 100*data;
		
		//cm730->ReadWord(int(platform->m_servo_IDs[i]),int(Robot::MX28::P_PRESENT_LOAD_L), &(data), 0); 			//<! Read Goal Position (target)
		addr = int(Robot::MX28::P_PRESENT_LOAD_L);
		data = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);
		joint[NUSensorsData::TorqueId] = data;
		//<! Current is blank
		joint[NUSensorsData::AccelerationId] = (joint[NUSensorsData::VelocityId] - m_previous_velocities[i])/delta_t;
		//<! Copy into m_data
		
		m_data->set(*m_joint_ids[i], m_current_time, joint);
        
        m_previous_positions[i] = joint[NUSensorsData::PositionId];
        m_previous_velocities[i] = joint[NUSensorsData::VelocityId];
	}
	delete datatable;
}

void DarwinSensors::copyFromAccelerometerAndGyro()
{
	//<! Get the data from the control board:
	int start_addr = int(Robot::CM730::P_GYRO_Z_L);
	int end_addr   = int(Robot::CM730::P_ACCEL_Z_H);
	int datasize   = end_addr-start_addr+1;
	unsigned char* datatable = new unsigned char[datasize];
	int error = 0;
	cm730->ReadTable(start_addr,end_addr,datatable,&error);
	
	int addr;
	int x,y,z;
	vector<float> data(3,0);
	//<! Assign the robot data to the NUSensor Structure:
	addr = int(Robot::CM730::P_GYRO_X_L);
	data[0] = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);

	addr = int(Robot::CM730::P_GYRO_Y_L);
	data[1] = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);
	
	addr = int(Robot::CM730::P_GYRO_Z_L);
	data[2] = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);
	
	m_data->set(NUSensorsData::Gyro,m_current_time, data);

	addr = int(Robot::CM730::P_ACCEL_X_L);
	data[0] = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);

	addr = int(Robot::CM730::P_ACCEL_Y_L);
	data[1] = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);
	
	addr = int(Robot::CM730::P_ACCEL_Z_L);
	data[2] = cm730->MakeWord(datatable[addr-start_addr],datatable[addr+1-start_addr]);
	
	m_data->set(NUSensorsData::Accelerometer,m_current_time, data);

	delete datatable;
}

void DarwinSensors::copyFromFeet()
{
	//Darwin has no feet sensors atm.
	return;
}

void DarwinSensors::copyFromButtons()
{
}

void DarwinSensors::copyFromBattery()
{
}
