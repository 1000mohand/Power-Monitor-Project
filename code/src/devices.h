#include <Arduino.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <style.hpp>
#include <list>
#include <map>
#include <array>


#define LED 2
#define SENSOR 36

typedef byte Pin;
typedef unsigned Read; //voltage read
struct pin_reads{
	short read [5] {};
	unsigned accum_reads = 0;
};



/**********************************************************************\
| The Class 'DevicesClass' :                                           |
|    Provides singletone interface to devices and handles monitoring   |
|and control over devices                                              |
\**********************************************************************/

class DevicesClass{
private:

    class Device;
    class ReadingClass;
    class Room;
    class Mainswitch;
    class Mux;

    DevicesClass() = default;
    DevicesClass(const DevicesClass&) = delete;

    static inline std::list<DevicesClass::Device> _devices_list;
    static inline std::list<DevicesClass::Room> _rooms_list;
    static inline std::list<DevicesClass::Mainswitch> 
            _mainswitches_list;
    static inline std::list<DevicesClass::Mux> _muxes_list;

    static inline std::map<String, std::list<DevicesClass::Device>
            ::iterator > _devices_map;
    static inline std::map<String, std::list<DevicesClass::Room>
            ::iterator> _rooms_map;
    static inline std::map<String, std::list<DevicesClass::
            Mainswitch>::iterator> _mainswitches_map;
    static inline std::map<String, std::list<DevicesClass::Mux>
            ::iterator> _muxes_map;   

    template < class Container>
    class Iterable_Range{
    private: 
        Container& container;
    public:
        Iterable_Range(Container& ctnr):container(ctnr){}
        typename Container::iterator begin(){
            return container.begin();
        }
        typename Container::iterator begin() const {
            return container.cbegin();
        }
        typename Container::iterator cbegin(){
            return container.cbegin();
        }
        typename Container::iterator end(){
            return container.end();
        }
        typename Container::iterator end() const {
            return container.cend();
        }
        typename Container::iterator cend(){
            return container.cend();
        }
        size_t size() const{
            return container.size();
        }
    };

    static std::list<DevicesClass::Room>::iterator 
            _addOrGetItr(const Room&); //get room by name
    static std::list<DevicesClass::Mainswitch>::iterator 
            _addOrGetItr(const Mainswitch&);//get mainswitch by name
    static std::list<DevicesClass::Device>::iterator 
            _addOrGetItr(const Device& device);//get device by name
    static std::list<DevicesClass::Mux>::iterator 
            _addOrGetItr(const Mux&);//get multiplexer by name

    static bool _changeRoom(Device&, const Room&);
    static bool _changeMstch(Device& device, const Mainswitch& mstch);
    
public:

    // Singletones are 'ere

    static DevicesClass Devices;
    static ReadingClass Reading;


    static void begin();
    
    // Element Access methods by map key

    static Device & device(const String& name){
        if (_devices_map.find(name) != _devices_map.end())
        return *_devices_map.at(name);
        else return *_devices_map.at("");
    }
    static Room & room(const String& name){
        if (_rooms_map.find(name) != _rooms_map.end())
        return *_rooms_map.at(name);
        else return *_rooms_map.at("");
    }
    static Mainswitch & mainswitch(const String& name){
        if (_mainswitches_map.find(name) != _mainswitches_map.end())
        return *_mainswitches_map.at(name);
        else return *_mainswitches_map.at("");
    }

    // Element Access methods by fast iteration

    static inline Iterable_Range devices_list = _devices_list;
    static inline Iterable_Range rooms_list = _rooms_list;
    static inline Iterable_Range mainswitches_list = _mainswitches_list;

    // Element modifiers

    static short get_device_voltage(DevicesClass::Device& dev);
};

// The Interface to control and monitor devices
inline DevicesClass& Devices = DevicesClass::Devices;


/**********************************************************************\
| The Class 'DevicesClass::ReadingClass' :                             |
|    Provides singletone interface to gather the reading of the devices|
|and event hookers to get notified on new reading events               |
\**********************************************************************/

class DevicesClass::ReadingClass{
private:
    struct Accumulated_Readings{
        const unsigned period=60;
        const EventBits_t event_bit = 0;
        unsigned counter=0;
    };
    static inline std::array<Accumulated_Readings,2> _acc_readings 
            {Accumulated_Readings{60  , 0b010, 0},
             Accumulated_Readings{1800, 0b100, 0}   };
    
    static void track_continious_readings (void*);

    friend class DevicesClass;

public:
    static inline EventGroupHandle_t reading_events 
            = xEventGroupCreate();
    static constexpr EventBits_t every_read_event     = 0b001;
    static constexpr EventBits_t frequent_reads_event = 0b010;
    static constexpr EventBits_t long_time_event      = 0b100;

    static constexpr unsigned every_read_index     = 0;
    static constexpr unsigned frequent_reads_index = 1;
    static constexpr unsigned long_time_index      = 2;
};


/**********************************************************************\
| The Class 'DevicesClass::Device' :                                   |
|   This class represents a devices connected to the ESP it hold all   |
|infromation about the device and the methods to access and modify     |
|device states                                                         |
\**********************************************************************/

class DevicesClass::Device{
    friend class ReadingClass;
    friend class DevicesClass;
    friend bool test2_loop_devices();

private:
    struct Access{
        enum  AccessType : uint {pin,mux,no_access,
                not_implemented=255,spi=not_implemented};

        union AccessDetails
        {
            Pin pin;
            struct {Pin pin; unsigned select; struct Mux* mux;}
                mux_access;
        };

        AccessType      read_type     = no_access;
        AccessDetails   read_details ;

        AccessType      write_type    = no_access;
        AccessDetails   write_details;
    };

    String _name;
    std::list<DevicesClass::Room>::iterator _room_itr;
    std::list<DevicesClass::Mainswitch>::iterator _mainswitch_itr;
    Access _access;
    Read _reads[5];
    std::array<Read,3> _accummulated_reads;    

public:
    Read getRead() const {
        Read sum = 0;
        for (const Read& read : _reads ){
            sum += read;
        }
        return sum/5;
    }
    Read getAccumulatedRead (unsigned index) const {
         return _accummulated_reads.at(index);
    }

    bool rename (const String& name){
        _name = name;
        return true;
    }
    const String& name(){
        return _name;
    }
    Room& room(){
        return *_room_itr;
    }
    Mainswitch& mainswitch(){
        return *_mainswitch_itr;
    }
    bool changeRoom (String);
    bool changeTo  (Room&);
    bool changeTo  (Mainswitch&);


};



class DevicesClass::Room{
    friend class DevicesClass;

private:
    std::list<DevicesClass::Device*> _devices_list;
    String _name;
    
    struct Iterator{
        typedef std::list<DevicesClass::Device*>::iterator
                    Iterator_t;
        typedef Iterator_t::difference_type difference_type;
        typedef Iterator_t::value_type value_type;
        typedef Iterator_t::pointer pointer;
        typedef Iterator_t::reference reference;
        typedef Iterator_t::iterator_category iterator_category;
        
        Iterator_t Itr;
        DevicesClass::Device& operator*(){
            return **Itr;
        }
        DevicesClass::Device* operator-> (){
            return *Itr;
        }
        Iterator operator++(int){
            Iterator ret = *this;
            Itr++;
            return ret;
        }
        Iterator& operator++(){
            Itr++;
            return *this;
        }
        bool operator==(const Iterator& other) {
            return this->Itr == other.Itr;
        }
        bool operator!=(const Iterator& other) {
            return this->Itr != other.Itr;
        }

        Iterator()=default;
        Iterator(const Iterator_t& Itr):Itr(Itr){};
    };

    struct Const_Iterator{
        typedef std::list<DevicesClass::Device*>::const_iterator
                    Iterator_t;
        typedef Iterator_t::difference_type difference_type;
        typedef Iterator_t::value_type value_type;
        typedef Iterator_t::pointer pointer;
        typedef Iterator_t::reference reference;
        typedef Iterator_t::iterator_category iterator_category;
        
        Iterator_t Itr;
        DevicesClass::Device& operator*(){
            return **Itr;
        }
        DevicesClass::Device* operator-> (){
            return *Itr;
        }
        Const_Iterator operator++(int){
            Const_Iterator ret = *this;
            Itr++;
            return ret;
        }
        Const_Iterator& operator++(){
            Itr++;
            return *this;
        }
        bool operator==(const Const_Iterator& other) const {
            return this->Itr == other.Itr;
        }
        bool operator!=(const Const_Iterator& other) const {
            return this->Itr != other.Itr;
        }

        Const_Iterator()=default;
        Const_Iterator(const Iterator_t& Itr):Itr(Itr){};
    };

public:
    Room()=default;
    Room(const Room&)=default;
    Room(Room&&)=default;
    Room& operator=(const Room&)=default;
    Room& operator=(Room &&)=default;
    Room(String name):_name(name){}
    Iterator begin(){
        return {_devices_list.begin()};
    }
    Iterator end(){
        return {_devices_list.end()};
    }
    Const_Iterator begin() const{
        return {_devices_list.cbegin()};
    }
    Const_Iterator end() const{
        return {_devices_list.cend()};
    }
    Const_Iterator cbegin() const{
        return {_devices_list.cbegin()};
    }
    Const_Iterator cend() const{
        return {_devices_list.cend()};
    }

    size_t count() const{
        return _devices_list.size();
    }
    const String& name() const {
        return _name;
    }
    void rename(String name){
        _name = name;
    }
 };



class DevicesClass::Mainswitch{
    friend class DevicesClass;

private:
    std::list<DevicesClass::Device*> _devices_list;
    String _name;
    
    struct Iterator{
        typedef std::list<DevicesClass::Device*>::iterator
                    Iterator_t;
        typedef Iterator_t::difference_type difference_type;
        typedef Iterator_t::value_type value_type;
        typedef Iterator_t::pointer pointer;
        typedef Iterator_t::reference reference;
        typedef Iterator_t::iterator_category iterator_category;
        
        Iterator_t Itr;
        DevicesClass::Device& operator*(){
            return **Itr;
        }
        DevicesClass::Device* operator-> (){
            return *Itr;
        }
        Iterator operator++(int){
            Iterator ret = *this;
            Itr++;
            return ret;
        }
        Iterator& operator++(){
            Itr++;
            return *this;
        }
        bool operator==(const Iterator& other) const {
            return this->Itr == other.Itr;
        }
        bool operator!=(const Iterator& other) const {
            return this->Itr != other.Itr;
        }

        Iterator()=default;
        Iterator(const Iterator_t& Itr):Itr(Itr){};
    };

    struct Const_Iterator{
        typedef std::list<DevicesClass::Device*>::const_iterator
                    Iterator_t;
        typedef Iterator_t::difference_type difference_type;
        typedef Iterator_t::value_type value_type;
        typedef Iterator_t::pointer pointer;
        typedef Iterator_t::reference reference;
        typedef Iterator_t::iterator_category iterator_category;
        
        Iterator_t Itr;
        DevicesClass::Device& operator*(){
            return **Itr;
        }
        DevicesClass::Device* operator-> (){
            return *Itr;
        }

        Const_Iterator operator++(int){
            Const_Iterator ret = *this;
            Itr++;
            return ret;
        }
        Const_Iterator& operator++(){
            Itr++;
            return *this;
        }
        bool operator==(const Const_Iterator& other) const {
            return this->Itr == other.Itr;
        }
        bool operator!=(const Const_Iterator& other) const {
            return this->Itr != other.Itr;
        }

        Const_Iterator()=default;
        Const_Iterator(const Iterator_t& Itr):Itr(Itr){};
    };

public:
    Mainswitch()=default;
    Mainswitch(const Mainswitch&)=default;
    Mainswitch(Mainswitch&&)=default;
    Mainswitch& operator=(const Mainswitch&)=default;
    Mainswitch& operator=(Mainswitch &&)=default;
    Mainswitch(String name):_name(name){}
    Iterator begin(){
        return {_devices_list.begin()};
    }
    Iterator end(){
        return {_devices_list.end()};
    }
    Const_Iterator begin() const{
        return {_devices_list.cbegin()};
    }
    Const_Iterator end() const{
        return {_devices_list.cend()};
    }
    Const_Iterator cbegin() const{
        return {_devices_list.cbegin()};
    }
    Const_Iterator cend() const{
        return {_devices_list.cend()};
    }

    size_t count() const{
        return _devices_list.size();
    }
    const String& name() const {
        return _name;
    }
    void rename(String name){
        _name = name;
    }
};

class DevicesClass::Mux{
    friend class DevicesClass;
    // TODO implement muxes
    String _name;
};


constexpr inline Pin  input_pins[] = {36, 39, 34, 35, 32, 33
                                     };  // All pins
//constexpr inline Pin  input_pins[] = {36, 39, 34, 35
//                                     };
constexpr inline Pin output_pins[] = {23, 22,  1,  3, 21, 19, 18,  5,
                                      14, 16,  4,  2, 15};

short get_device_voltage(Pin pin);