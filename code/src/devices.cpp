#include <Arduino.h>
#include <freertos/task.h>
#include <SPIFFS.h>
#include <map>
#include <stack>

#include <pugixml.hpp>
#include <style.hpp>
#include "literals"

#include "devices.h"

std::list<DevicesClass::Room>::iterator 
        DevicesClass::_addOrGetItr(const Room& room){
        auto map_itr = _rooms_map.find(room._name);
    if (map_itr == _rooms_map.end()){ 
    //means this room is new. Hence, we add it to the list
        _rooms_list.push_back(room);
        auto itr = std::prev(_rooms_list.end());
        _rooms_map.insert_or_assign(room._name,itr);
        return itr;
    }
    else return (map_itr->second);
}

std::list<DevicesClass::Mainswitch>::iterator 
        DevicesClass::_addOrGetItr(const Mainswitch& mstch){
    auto map_itr = _mainswitches_map.find(mstch._name);
    if (map_itr == _mainswitches_map.end()){ 
    //means this switch is new. Hence, we add it to the list
        _mainswitches_list.push_back(mstch);
        auto itr = std::prev(_mainswitches_list.end());
        _mainswitches_map.insert_or_assign(mstch._name,itr);
        return itr;
    }
    else return (map_itr->second);
}

std::list<DevicesClass::Device>::iterator
        DevicesClass::_addOrGetItr(const Device& device){\
    auto map_itr = _devices_map.find(device._name);
    if (map_itr == _devices_map.end()){ 
    //means this device is new. Hence, we add it to the list
        _devices_list.push_back(device);
        auto itr = std::prev(_devices_list.end());
        _devices_map.insert_or_assign(device._name,itr);
        return itr;
    }
    else return (map_itr->second);
}

std::list<DevicesClass::Mux>::iterator 
        DevicesClass::_addOrGetItr(const Mux& mux){
        auto map_itr = _muxes_map.find(mux._name);
    if (map_itr == _muxes_map.end()){ 
    //means this room is new. Hence, we add it to the list
        _muxes_list.push_back(mux);
        auto itr = std::prev(_muxes_list.end());
        _muxes_map.insert_or_assign(mux._name,itr);
        return itr;
    }
    else return (map_itr->second);
}

bool DevicesClass::_changeRoom(Device& device, const Room& room){
    if (device._room_itr != _addOrGetItr(room)){
        auto room_itr = _addOrGetItr(room);
        device._room_itr->_devices_list.remove(&device);
        device._room_itr = room_itr;
        room_itr->_devices_list.push_back(&device);
        return true;
    }
    else return false;
}
bool DevicesClass::_changeMstch(Device& device, const Mainswitch& mstch){
    if (device._mainswitch_itr != _addOrGetItr(mstch)){
        auto mainswitch_itr = _addOrGetItr(mstch);
        device._mainswitch_itr->_devices_list.remove(&device);
        device._mainswitch_itr = mainswitch_itr;
        mainswitch_itr->_devices_list.push_back(&device);
        return true;
    }
    else return false;
}


/*bool DevicesClass::_changeRoom(Device& device, Room& room){
    if (device._room_itr != _addOrGetItr(room)){
        
    }
}*/

void DevicesClass::begin(){

#ifdef DEBUG
    Serial.println("\nmaking devices");
#endif
    pugi::xml_document doc;
    fs::File file = SPIFFS.open("/devices.xml");
    size_t file_size = file.size();

    char* doc_text = new char [file_size];

    file.readBytes(doc_text,file_size);
    file.close();
    pugi::xml_parse_result result = doc.load_buffer(doc_text,file_size);
    delete[] doc_text;

#ifdef DEBUG
    Serial.println("state : ");
    Serial.println(result.operator bool() );
    Serial.print("\t");
    Serial.println(result.description()) ;
    Serial.println("getting a peek on the list\n");

    for (const decltype(auto) node : doc){
    Serial.println(node.name());
    for (const decltype(auto) subnode : node){
    Serial.println(String(".\t") + subnode.name()) ;
    for (const decltype(auto) subnode : subnode){
    Serial.println(String(".\t\t") + (subnode.name()));
    /*for (const decltype(auto) subnode : subnode)
    Serial.println"--: " << (subnode.name()) << ": " <<
            subnode.value() << subnode.first_child().value() << "\n";
    */}}}
#endif


    struct DataExtracter{
        std::stack<Device> devices;
        void start_extraction(pugi::xml_document& doc)
        { 
#ifdef DEBUG
            Serial.println("starting extraction");
#endif
            using namespace std::string_literals;

            // empty rooms and mainswitches for non-associated devices
            // put in the first device in the stack to copied later.

            if (devices.size() > 0)
            then devices.push(devices.top());
            else devices.push(Device());
            Device& device = devices.top();

            device._room_itr = _addOrGetItr(Room());
            device._mainswitch_itr = _addOrGetItr(Mainswitch());
            //this contains only one device, the null device, out of 
            //the other lists to prevent appearing while iterating 
            //the other lists.
            static std::list<Device> null_device_list {Device()};
            _devices_map.insert({"",null_device_list.begin()});


            // start iterating the xml

            Serial.println("iterating xml");

            for (auto entery   : doc)
            if (entery.name() == "devices"s)
            for (auto node     : entery){
                if      (node.name() == "muxs"s){
                    for (auto node : node)
                    if  (node.name() == "mux"s)
                        extract_mux(node);
                }
                else if (node.name() == "rooms"s){
                    for (auto node : node)
                    if  (node.name() == "room"s)
                        extract_room(node);
                }
                else if (node.name() == "mainswitches"s){
                    for (auto node : node)
                    if  (node.name() == "mainswitch"s)
                        extract_mstch(node);
                }
                else if (node.name() == "room"s)
                then extract_room(node);
                else if (node.name() == "mainswitch"s)
                then extract_mstch(node);
                else if (node.name() == "device"s)
                then extract_device(node);
            }
            devices.pop();
        }

        void extract_device(pugi::xml_node node)
        {
             using namespace std::string_literals;
            
#ifdef DEBUG
            Serial.println("device");
            if (node.name() != "device"s) {
                throw(std::runtime_error(
                    ("Error constructing a device from: "s 
                     + node.name()
                    ).c_str())
                );
            }
#endif 
                if (node.name() != "device"s) return;
            

            // stack preparation

            if (devices.size() > 0)
            then devices.push(devices.top());
            else devices.push(Device());
  
            // info extraction

            Device& device = devices.top();

            for (auto subnode : node){
                if      (subnode.name() == "name"s){
                    if (subnode.first_child().type() ==
                            pugi::node_pcdata)
                    then device.rename(subnode.first_child().value());
                }
                else if (subnode.name() == "room"s){
                    if (subnode.first_child().type() ==
                            pugi::node_pcdata){
                        device._room_itr = _addOrGetItr(
                                Room(subnode.first_child().value()));
                    }
                }
                else if (subnode.name() == "mainswitch"s){
                    if (subnode.first_child().type() ==
                            pugi::node_pcdata){
                        device._mainswitch_itr = _addOrGetItr(Mainswitch
                                (subnode.first_child().value()));
                    }
                }
                else if (subnode.name() == "access"s)
                then extract_access(subnode);
            }

            // register info from the stack to the list

            std::list<DevicesClass::Device>::iterator itr;
            itr = _addOrGetItr(device);
            device.room()._devices_list.push_back(&*itr);
            device.mainswitch()._devices_list.push_back(&*itr);

            // pop stack
            devices.pop();
        }
        void extract_mux   (pugi::xml_node node)
        {
            
        }
        void extract_room  (pugi::xml_node node)
        {
            using namespace std::string_literals;
            
#ifdef DEBUG
            if (node.name() != "room"s) {
                throw(std::runtime_error(
                    ("Error constructing a room from: "s 
                     + node.name()
                    ).c_str())
                );
            }
#endif          
                if (node.name() != "room"s)
                return;

            // stack preparation

            if (devices.size() > 0)
            then devices.push(devices.top());
            else devices.push(Device());

            // info extraction

            Room room;

            for (auto subnode : node){
                if (subnode.name() == "name"s)
                if (subnode.first_child().type() == pugi::node_pcdata)
                then room.rename(subnode.first_child().value());
            }


            // register info

            std::list<DevicesClass::Room>::iterator itr ;
            itr = _addOrGetItr(room);
            *itr = room;

            // configure stack device

            devices.top()._room_itr = itr;

            // ready to extract sub elements

            for (auto subnode : node){
                if      (subnode.name() == "mainswitch"s)
                then extract_mstch(subnode);
                else if (subnode.name() == "device"s)
                then extract_device(subnode);
            }

            // pop stack
            devices.pop();
        }
        void extract_mstch(pugi::xml_node node)
        {
            using namespace std::string_literals;
#ifdef DEBUG
                Serial.println("mainswitch");
                if (node.name() != "mainswitch"s) {
                throw(std::runtime_error(
                    ("Error constructing a mainswitch from: "s 
                     + node.name()
                    ).c_str())
                );
#endif
                if (node.name() != "mainswitch"s)   return;
            }

            // stack preparation

            if (devices.size() > 0)
            then devices.push(devices.top());
            else devices.push(Device());

            // info extraction

            Mainswitch mstch;

            for (auto subnode : node){
                if (subnode.name() == "name"s)
                if (subnode.first_child().type() == pugi::node_pcdata)
                then mstch.rename(subnode.first_child().value());
            }


            // register info

            std::list<DevicesClass::Mainswitch>::iterator itr ;
            itr = _addOrGetItr(mstch);
            *itr = mstch;

            // configure stack device

            devices.top()._mainswitch_itr = itr;

            // ready to extract sub elements

            for (auto subnode : node){
                if      (subnode.name() == "room"s)
                then extract_room(subnode);
                else if (subnode.name() == "device"s)
                then extract_device(subnode);
            }

            // pop stack
            devices.pop();
        }
        void extract_access(pugi::xml_node node){

            using namespace std::string_literals;
            
#ifdef DEBUG
                Serial.println("access");
                if (node.name() != "access"s) {
                throw(std::runtime_error(
                    ("Error constructing a access from: "s 
                     + node.name()
                    ).c_str())
                );
                if (devices.size() != 0){
                     throw(std::runtime_error("Error stack is empty! "
                            "no previous device to manipulate"));
                }
#endif          
                if ((node.name() != "access"s) || (devices.size() != 0))
                then return;
            }

            // info extraction

            Device& device = devices.top();
#ifdef DEBUG
            Serial.println(".\treading access info");
#endif
            for (auto subnode : node){
                if (subnode.name() == "pin"s){
                    auto child = subnode.first_child();
                    if (child.type() == pugi::node_pcdata){
                        device._access.read_type = Device::Access::pin;
                        String s = child.value();
                        auto comma_index = s.indexOf(",");
                        if (comma_index == -1){
                            device._access.read_details.pin = 
                                    s.toInt();
                        }
                        else {
                            device._access.read_details.pin = 
                                s.substring(0,comma_index-1).toInt();
                            device._access.write_type = Device::
                                    Access::pin;
                            device._access.write_details.pin = 
                                (s.substring(comma_index+1)).toInt();
                        }
                    }
                    else for (auto subnode : subnode){
                        if      (subnode.name() == "read"s){
                            device._access.read_type = Device::
                                Access::pin; 
                            device._access.read_details.pin = 
                                std::stoi(subnode.child_value());
                        }
                        else if (subnode.name() == "control"s){
                            device._access.write_type = Device::
                                Access::pin; 
                            device._access.write_details.pin = 
                                std::stoi(subnode.child_value());
                        }
#ifdef DEBUG
                        else throw std::runtime_error("Error "
                            "cant undersatand"s + subnode.name() +
                            " " + subnode.child_value());
#endif
                    }
                }
                else if (subnode.name() == "read"s){
#ifdef  DEBUG
                    Serial.println(".\ttrying to access->read");
#endif
                    auto child = subnode.first_child();
                    if      (child.name() == "pin"s){
                        if (child.first_child().type() 
                                            == pugi::node_pcdata){
                            device._access.read_type = 
                                    Device::Access::pin;
                            String s = child.child_value();
                            device._access.read_details.pin = 
                                        s.toInt();
                        }
                    }
                    else if      (child.name() == "mux"s){
                        //TODO implement mux
                        device._access.read_type = Device::Access::mux;
                    }
                }
                else if (subnode.name() == "control"s){
                    auto child = subnode.first_child();
                    if      (child.name() == "pin"s){
                        if (child.first_child().type() 
                                            == pugi::node_pcdata){
                            device._access.write_type = 
                                    Device::Access::pin;
                            String s = child.child_value();
                            device._access.write_details.pin = 
                                        s.toInt();
                        }
                    }
                    else if      (child.name() == "mux"s){
                        //TODO implement mux
                        device._access.write_type = Device::Access::mux;
                    }
                }
                else if (subnode.name() == "mux"s)
                // TODO implement mux
                ;
            }
        }
    };

    DataExtracter().start_extraction(doc);

    xTaskCreate(Reading.track_continious_readings,"readings",1280,
                NULL,2,NULL);
}

void DevicesClass::ReadingClass::track_continious_readings(void*){

    for (auto& acc_reading : _acc_readings){
        if (not (++acc_reading.counter < acc_reading.period)){
            acc_reading.counter = 0;
            xEventGroupSetBits(reading_events,acc_reading.event_bit);
        }
    }
    xEventGroupSetBits(reading_events,every_read_event);

    for (auto& device : Devices._devices_list){
        Read read;

        if (device._access.read_type == Device::Access::pin){
            repeat_and_index(4,i){
                device._reads[i]= device._reads[i+1];
            }
            Pin& device_pin = device._access.read_details.pin;
            read = analogRead(device_pin);
            device._reads[4] = read;
        }
        else{
#ifdef DEBUG
            Serial.print("Error: read method not implemented yet!");
#endif
            read = 0;
        }

        for (size_t i = 0; i<_acc_readings.size(); i++){
            if (_acc_readings[i].counter == 1){
                device._accummulated_reads[i] = 0;
            }
            device._accummulated_reads[i] += read;
        }
    }
}