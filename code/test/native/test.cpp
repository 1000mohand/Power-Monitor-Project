#include <iostream>
#include <pugixml.hpp>
#include "devices.h"

#include <unity.h>


bool test1_beginning();
bool test2_loop_devices();
bool test3_loop_rooms();
bool test4_loop_mainswitches();

int main(){
	UNITY_BEGIN();
	std::cout << __cplusplus;
	TEST_ASSERT(test1_beginning());	
	TEST_ASSERT(test2_loop_devices());
	TEST_ASSERT(test3_loop_rooms());
	TEST_ASSERT( test4_loop_mainswitches());
	UNITY_END();
}


bool test1_beginning (){
	Devices.begin();
	std::cout << "devices made/constructed with no thrown errors\n\n";
	return true;
}

bool test2_loop_devices(){
	std::cout << "\nlisting Devices:" << Devices.devices_list.size() 
			  << "\n\n";
	for (auto& device : Devices.devices_list){
		std::cout << "name: " << device.name() << "\n"; 
		std::cout << "room: " << device.room().name() << "\n";
		std::cout << "mswtch: " << device.mainswitch().name() << "\n";

		std::cout << "access:- \n";
		std::cout << "read access type: ";
		if 		(device._access.read_type == device._access.pin){
			std::cout << "pin: " << device._access.read_details.pin;
		}
		else if (device._access.read_type == device._access.mux){
			std::cout << "mux: ??""!";
		}
		else if (device._access.read_type == device._access.spi){
			std::cout << "spi: ??""!";
		}
		else if (device._access.read_type == device._access.no_access){
			std::cout << "no access";
		}
		else{
			std::cout << "Error read access undefined!\n";
			return false;
		}


		std::cout << "\ncontrol access type: ";
		if 		(device._access.write_type == device._access.pin){
			std::cout << "pin: " << device._access.write_details.pin 
					  << "\n";
		}
		else if (device._access.write_type == device._access.mux){
			std::cout << "mux: ??""!";
		}
		else if (device._access.write_type == device._access.spi){
			std::cout << "spi: ??""!";
		}
		else if (device._access.write_type == device._access.no_access){
			std::cout << "no access";
		}
		else{
			std::cout << "Error read access undefined!\n";
			return false;
		}
		std::cout << "\n\n";
	}
	std::cout << "\n\n";
	return true;
}

bool test3_loop_rooms(){
	std::cout << "\nlisting Rooms:" << Devices.rooms_list.size() 
			  << "\n\n";
	for (auto& room : Devices.rooms_list){
		std::cout << "name: " << room.name() << "\n";
		std::cout << "listing devices:- " << room.count() << "\n";
		for (auto device : room){
			std::cout << ".\tname: " << device.name() << "\n"; 
			std::cout << ".\troom: " << device.room().name() << "\n";
			std::cout << ".\tmswtch: " << device.mainswitch().name();
			std::cout << "\n.\n";
		}
		std::cout << "\n";
	}
	std::cout << "\n\n";
	return true;
}

bool test4_loop_mainswitches (){
	std::cout << "\nlisting Mainswitches:" 
			  << Devices.mainswitches_list.size()  << "\n\n";
	for (auto& mstch : Devices.mainswitches_list){
		std::cout << "name: " << mstch.name() << "\n";
		std::cout << "listing devices:- " << mstch.count() << "\n";
		for (auto device : mstch){
			std::cout << ".\tname: " << device.name() << "\n"; 
			std::cout << ".\troom: " << device.room().name() << "\n";
			std::cout << ".\tmswtch: " << device.mainswitch().name();
			std::cout << "\n.\n";
		}
		std::cout << "\n";
	}
	std::cout << "\n\n";
	return true;
}