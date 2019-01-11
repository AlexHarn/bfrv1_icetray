#include "SmallShowerGeometries.h"

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

#include <map>


namespace smallshower_filter {

  /*
   * Combinations of 3 stations, IC40
   */
  const StationList* getStation3_IC40()
  {
    static StationList *stations = 0;

    if (!stations) {
      stations = new StationList(0);
      *stations +=
	list_of(61)(62)(69),
	list_of(62)(69)(70),
	list_of(62)(63)(70),
	list_of(54)(62)(63),
	list_of(63)(70)(71),
	list_of(63)(64)(71),
	list_of(54)(55)(63),
	list_of(55)(63)(64),
	list_of(55)(56)(64),
	list_of(56)(64)(65),
	list_of(56)(57)(65),
	list_of(57)(65)(66),
	list_of(57)(58)(66),
	list_of(48)(57)(58),
	list_of(48)(49)(58),
	list_of(39)(48)(49);
    }

    return stations;
  }

  /*
   * Combinations of 3 stations, IC59
   */
  const StationList* getStation3_IC59()
  {
    static StationList *stations = 0;

    if (!stations) {
      stations = new StationList(0);
      *stations +=
	list_of(61)(62)(69),
	list_of(62)(69)(70),
	list_of(62)(63)(70),
	list_of(54)(62)(63),
	list_of(63)(70)(71),
	list_of(63)(64)(71),
	list_of(54)(55)(63),
	list_of(55)(63)(64),
	list_of(55)(56)(64),
	list_of(56)(64)(65),
	list_of(56)(57)(65),
	list_of(57)(65)(66),
	list_of(57)(58)(66),
	list_of(48)(57)(58),
	list_of(48)(49)(58),
	list_of(39)(48)(49),
	list_of(46)(55)(56),
	list_of(46)(47)(56),
	list_of(47)(56)(57),
	list_of(47)(48)(57),
	list_of(38)(39)(48),
	list_of(38)(47)(48),
	list_of(37)(38)(47),
	list_of(37)(46)(47),
	list_of(29)(38)(39),
	list_of(28)(29)(38),
	list_of(28)(37)(38),
	list_of(27)(28)(37),
	list_of(18)(19)(27),
	list_of(19)(27)(28),
	list_of(19)(20)(28),
	list_of(20)(28)(29),
	list_of(10)(11)(18),
	list_of(11)(18)(19),
	list_of(11)(12)(19),
	list_of(12)(19)(20);

       
    }

    return stations;
  }


   /*
   * Combinations of 3 stations, IC79
   */
  const StationList* getStation3_IC79()
  {
    static StationList *stations = 0;

    if (!stations) {
      stations = new StationList(0);
      *stations +=
	list_of(61)(62)(69),
	list_of(62)(69)(70),
	list_of(62)(63)(70),
	list_of(54)(62)(63),
	list_of(63)(70)(71),
	list_of(63)(64)(71),
	list_of(54)(55)(63),
	list_of(55)(63)(64),
	list_of(55)(56)(64),
	list_of(56)(64)(65),
	list_of(56)(57)(65),
	list_of(57)(65)(66),
	list_of(57)(58)(66),
	list_of(48)(57)(58),
	list_of(48)(49)(58),
	list_of(39)(48)(49),
	list_of(46)(55)(56),
	list_of(46)(47)(56),
	list_of(47)(56)(57),
	list_of(47)(48)(57),
	list_of(38)(39)(48),
	list_of(38)(47)(48),
	list_of(37)(38)(47),
	list_of(37)(46)(47),
	list_of(29)(38)(39),
	list_of(28)(29)(38),
	list_of(28)(37)(38),
	list_of(27)(28)(37),
	list_of(18)(19)(27),
	list_of(19)(27)(28),
	list_of(19)(20)(28),
	list_of(20)(28)(29),
	list_of(10)(11)(18),
	list_of(11)(18)(19),
	list_of(11)(12)(19),
	list_of(12)(19)(20),
	list_of(61)(52)(53),
	list_of(61)(62)(53),
	list_of(62)(53)(54),
	list_of(52)(42)(43),
	list_of(52)(53)(43),
	list_of(53)(43)(44),
	list_of(53)(54)(44),
	list_of(54)(44)(45),
	list_of(54)(55)(45),
	list_of(55)(45)(46),
	list_of(42)(43)(33),
	list_of(43)(33)(34),
	list_of(43)(44)(34),
	list_of(44)(34)(35),
	list_of(44)(45)(35),
	list_of(45)(35)(36),
	list_of(45)(46)(36),
	list_of(46)(36)(37),
	list_of(33)(34)(24),
	list_of(34)(24)(25),
	list_of(34)(35)(25),
	list_of(35)(25)(26),
	list_of(35)(36)(26),
	list_of(36)(26)(27),
	list_of(36)(37)(27),
	list_of(24)(25)(16),
	list_of(25)(16)(17),
	list_of(25)(26)(17),
	list_of(26)(17)(18),
	list_of(26)(27)(18),
	list_of(17)(18)(10),
	list_of(17)(9)(10),
	list_of(16)(17)(9);
       
    }

    return stations;
  }


  /*
   * Combinations of 3 stations, IC86
   */
  const StationList* getStation3_IC86()
  {
    static StationList *stations = 0;

    if (!stations) {
      stations = new StationList(0);
      *stations +=
	list_of(61)(62)(69),
	list_of(62)(69)(70),
	list_of(62)(63)(70),
	list_of(54)(62)(63),
	list_of(63)(70)(71),
	list_of(63)(64)(71),
	list_of(54)(55)(63),
	list_of(55)(63)(64),
	list_of(55)(56)(64),
	list_of(56)(64)(65),
	list_of(56)(57)(65),
	list_of(57)(65)(66),
	list_of(57)(58)(66),
	list_of(48)(57)(58),
	list_of(48)(49)(58),
	list_of(39)(48)(49),
	list_of(46)(55)(56),
	list_of(46)(47)(56),
	list_of(47)(56)(57),
	list_of(47)(48)(57),
	list_of(38)(39)(48),
	list_of(38)(47)(48),
	list_of(37)(38)(47),
	list_of(37)(46)(47),
	list_of(29)(38)(39),
	list_of(28)(29)(38),
	list_of(28)(37)(38),
	list_of(27)(28)(37),
	list_of(18)(19)(27),
	list_of(19)(27)(28),
	list_of(19)(20)(28),
	list_of(20)(28)(29),
	list_of(10)(11)(18),
	list_of(11)(18)(19),
	list_of(11)(12)(19),
	list_of(12)(19)(20),
	list_of(61)(52)(53),
	list_of(61)(62)(53),
	list_of(62)(53)(54),
	list_of(52)(42)(43),
	list_of(52)(53)(43),
	list_of(53)(43)(44),
	list_of(53)(54)(44),
	list_of(54)(44)(45),
	list_of(54)(55)(45),
	list_of(55)(45)(46),
	list_of(42)(43)(33),
	list_of(43)(33)(34),
	list_of(43)(44)(34),
	list_of(44)(34)(35),
	list_of(44)(45)(35),
	list_of(45)(35)(36),
	list_of(45)(46)(36),
	list_of(46)(36)(37),
	list_of(33)(34)(24),
	list_of(34)(24)(25),
	list_of(34)(35)(25),
	list_of(35)(25)(26),
	list_of(35)(36)(26),
	list_of(36)(26)(27),
	list_of(36)(37)(27),
	list_of(24)(25)(16),
	list_of(25)(16)(17),
	list_of(25)(26)(17),
	list_of(26)(17)(18),
	list_of(26)(27)(18),
	list_of(17)(18)(10),
	list_of(17)(9)(10),
	list_of(16)(17)(9),
	list_of(32)(33)(42),
	list_of(32)(33)(23),
	list_of(23)(24)(33),
	list_of(23)(24)(15),
	list_of(15)(16)(24),
	list_of(15)(16)(8),
	list_of(8)(9)(16);
       
    }

    return stations;
  }




  /*
   * Combinations of 4 stations, IC40
   */
  const StationList* getStation4_IC40()
  {
    static StationList *stations = 0;

    if (!stations) {
      stations = new StationList(0);
      *stations +=
	list_of(61)(62)(69)(70),
	list_of(62)(63)(69)(70),
	list_of(54)(62)(63)(70),
	list_of(62)(63)(70)(71),
	list_of(63)(64)(70)(71),
	list_of(54)(55)(62)(63),
	list_of(55)(63)(64)(71),
	list_of(54)(55)(63)(64),
	list_of(55)(56)(63)(64),
	list_of(55)(56)(64)(65),
	list_of(56)(57)(64)(65),
	list_of(56)(57)(65)(66),
	list_of(57)(58)(65)(66),
	list_of(48)(57)(58)(66),
	list_of(48)(49)(57)(58),
	list_of(39)(48)(49)(58);
    }

    return stations;
  }


  /*
   * Combinations of 4 stations, IC59
   */
  const StationList* getStation4_IC59()
  {
    static StationList *stations = 0;

    if (!stations) {
      stations = new StationList(0);
      *stations +=
	list_of(61)(62)(69)(70),
	list_of(62)(63)(69)(70),
	list_of(54)(62)(63)(70),
	list_of(62)(63)(70)(71),
	list_of(63)(64)(70)(71),
	list_of(54)(55)(62)(63),
	list_of(55)(63)(64)(71),
	list_of(54)(55)(63)(64),
	list_of(55)(56)(63)(64),
	list_of(55)(56)(64)(65),
	list_of(56)(57)(64)(65),
	list_of(56)(57)(65)(66),
	list_of(57)(58)(65)(66),
	list_of(48)(57)(58)(66),
	list_of(48)(49)(57)(58),
	list_of(39)(48)(49)(58),
	list_of(46)(55)(56)(64),
	list_of(47)(56)(57)(65),
	list_of(46)(47)(55)(56),
	list_of(46)(47)(56)(57),
	list_of(47)(48)(56)(57),
	list_of(47)(48)(57)(58),
	list_of(38)(39)(48)(49),
	list_of(37)(46)(47)(56),
	list_of(38)(47)(48)(57),
	list_of(38)(39)(47)(48),
	list_of(37)(38)(47)(48),
	list_of(37)(38)(46)(47),
	list_of(29)(38)(39)(48),
	list_of(28)(37)(38)(47),
	list_of(28)(29)(38)(39),
	list_of(28)(29)(37)(38),
	list_of(20)(28)(29)(38),
	list_of(27)(28)(37)(38),
	list_of(19)(27)(28)(37),
	list_of(19)(20)(28)(29),
	list_of(19)(20)(27)(28),
	list_of(12)(19)(20)(28),
	list_of(18)(19)(27)(28),
	list_of(11)(12)(19)(20),
	list_of(11)(18)(19)(27),
	list_of(11)(12)(18)(19),
	list_of(10)(11)(18)(19);
      
    
    }

    return stations;
  }


  /*
   * Combinations of 4 stations, IC79
   */
  const StationList* getStation4_IC79()
  {
    static StationList *stations = 0;

    if (!stations) {
      stations = new StationList(0);
      *stations +=
	list_of(61)(62)(69)(70),
	list_of(62)(63)(69)(70),
	list_of(54)(62)(63)(70),
	list_of(62)(63)(70)(71),
	list_of(63)(64)(70)(71),
	list_of(54)(55)(62)(63),
	list_of(55)(63)(64)(71),
	list_of(54)(55)(63)(64),
	list_of(55)(56)(63)(64),
	list_of(55)(56)(64)(65),
	list_of(56)(57)(64)(65),
	list_of(56)(57)(65)(66),
	list_of(57)(58)(65)(66),
	list_of(48)(57)(58)(66),
	list_of(48)(49)(57)(58),
	list_of(39)(48)(49)(58),
	list_of(46)(55)(56)(64),
	list_of(47)(56)(57)(65),
	list_of(46)(47)(55)(56),
	list_of(46)(47)(56)(57),
	list_of(47)(48)(56)(57),
	list_of(47)(48)(57)(58),
	list_of(38)(39)(48)(49),
	list_of(37)(46)(47)(56),
	list_of(38)(47)(48)(57),
	list_of(38)(39)(47)(48),
	list_of(37)(38)(47)(48),
	list_of(37)(38)(46)(47),
	list_of(29)(38)(39)(48),
	list_of(28)(37)(38)(47),
	list_of(28)(29)(38)(39),
	list_of(28)(29)(37)(38),
	list_of(20)(28)(29)(38),
	list_of(27)(28)(37)(38),
	list_of(19)(27)(28)(37),
	list_of(19)(20)(28)(29),
	list_of(19)(20)(27)(28),
	list_of(12)(19)(20)(28),
	list_of(18)(19)(27)(28),
	list_of(11)(12)(19)(20),
	list_of(11)(18)(19)(27),
	list_of(11)(12)(18)(19),
	list_of(10)(11)(18)(19),
        list_of(69)(61)(62)(53),
        list_of(61)(62)(52)(53),
        list_of(61)(62)(53)(54),
        list_of(62)(63)(53)(54),
        list_of(61)(52)(53)(43),
        list_of(62)(53)(54)(44),
        list_of(63)(54)(55)(45),
        list_of(52)(53)(42)(43),
        list_of(52)(53)(43)(44),
        list_of(53)(54)(43)(44),
        list_of(53)(54)(44)(45),
        list_of(54)(55)(44)(45),
        list_of(54)(55)(45)(46),
        list_of(55)(56)(45)(46),
        list_of(52)(42)(43)(33),
        list_of(53)(43)(44)(34),
        list_of(54)(44)(45)(35),
        list_of(55)(45)(46)(36),
        list_of(46)(47)(36)(37),
        list_of(45)(46)(36)(37),
        list_of(45)(46)(35)(36),
        list_of(44)(45)(35)(36),
        list_of(44)(45)(34)(35),
        list_of(43)(44)(34)(35),
        list_of(43)(44)(33)(34),
        list_of(42)(43)(33)(34),
        list_of(43)(33)(34)(24),
        list_of(44)(34)(35)(25),
        list_of(45)(35)(36)(26),
        list_of(46)(36)(37)(27),
        list_of(36)(37)(27)(28),
        list_of(36)(37)(26)(27),
        list_of(35)(36)(26)(27),
        list_of(35)(36)(25)(26),
        list_of(34)(35)(25)(26),
        list_of(34)(35)(24)(25),
        list_of(33)(34)(24)(25),
        list_of(34)(24)(25)(16),
        list_of(35)(25)(26)(17),
        list_of(36)(26)(27)(18),
        list_of(26)(27)(18)(19),
        list_of(26)(27)(17)(18),
        list_of(25)(26)(17)(18),
        list_of(25)(26)(16)(17),
        list_of(24)(25)(16)(17),
        list_of(25)(16)(17)(9),
        list_of(26)(17)(18)(10),
        list_of(17)(18)(10)(11),
        list_of(17)(18)(9)(10),
        list_of(16)(17)(9)(10);
      
    
    }

    return stations;
  }


  /*
   * Combinations of 4 stations, IC86
   */
  const StationList* getStation4_IC86()
  {
    static StationList *stations = 0;

    if (!stations) {
      stations = new StationList(0);
      *stations +=
	list_of(61)(62)(69)(70),
	list_of(62)(63)(69)(70),
	list_of(54)(62)(63)(70),
	list_of(62)(63)(70)(71),
	list_of(63)(64)(70)(71),
	list_of(54)(55)(62)(63),
	list_of(55)(63)(64)(71),
	list_of(54)(55)(63)(64),
	list_of(55)(56)(63)(64),
	list_of(55)(56)(64)(65),
	list_of(56)(57)(64)(65),
	list_of(56)(57)(65)(66),
	list_of(57)(58)(65)(66),
	list_of(48)(57)(58)(66),
	list_of(48)(49)(57)(58),
	list_of(39)(48)(49)(58),
	list_of(46)(55)(56)(64),
	list_of(47)(56)(57)(65),
	list_of(46)(47)(55)(56),
	list_of(46)(47)(56)(57),
	list_of(47)(48)(56)(57),
	list_of(47)(48)(57)(58),
	list_of(38)(39)(48)(49),
	list_of(37)(46)(47)(56),
	list_of(38)(47)(48)(57),
	list_of(38)(39)(47)(48),
	list_of(37)(38)(47)(48),
	list_of(37)(38)(46)(47),
	list_of(29)(38)(39)(48),
	list_of(28)(37)(38)(47),
	list_of(28)(29)(38)(39),
	list_of(28)(29)(37)(38),
	list_of(20)(28)(29)(38),
	list_of(27)(28)(37)(38),
	list_of(19)(27)(28)(37),
	list_of(19)(20)(28)(29),
	list_of(19)(20)(27)(28),
	list_of(12)(19)(20)(28),
	list_of(18)(19)(27)(28),
	list_of(11)(12)(19)(20),
	list_of(11)(18)(19)(27),
	list_of(11)(12)(18)(19),
	list_of(10)(11)(18)(19),
        list_of(69)(61)(62)(53),
        list_of(61)(62)(52)(53),
        list_of(61)(62)(53)(54),
        list_of(62)(63)(53)(54),
        list_of(61)(52)(53)(43),
        list_of(62)(53)(54)(44),
        list_of(63)(54)(55)(45),
        list_of(52)(53)(42)(43),
        list_of(52)(53)(43)(44),
        list_of(53)(54)(43)(44),
        list_of(53)(54)(44)(45),
        list_of(54)(55)(44)(45),
        list_of(54)(55)(45)(46),
        list_of(55)(56)(45)(46),
        list_of(52)(42)(43)(33),
        list_of(53)(43)(44)(34),
        list_of(54)(44)(45)(35),
        list_of(55)(45)(46)(36),
        list_of(46)(47)(36)(37),
        list_of(45)(46)(36)(37),
        list_of(45)(46)(35)(36),
        list_of(44)(45)(35)(36),
        list_of(44)(45)(34)(35),
        list_of(43)(44)(34)(35),
        list_of(43)(44)(33)(34),
        list_of(42)(43)(33)(34),
        list_of(43)(33)(34)(24),
        list_of(44)(34)(35)(25),
        list_of(45)(35)(36)(26),
        list_of(46)(36)(37)(27),
        list_of(36)(37)(27)(28),
        list_of(36)(37)(26)(27),
        list_of(35)(36)(26)(27),
        list_of(35)(36)(25)(26),
        list_of(34)(35)(25)(26),
        list_of(34)(35)(24)(25),
        list_of(33)(34)(24)(25),
        list_of(34)(24)(25)(16),
        list_of(35)(25)(26)(17),
        list_of(36)(26)(27)(18),
        list_of(26)(27)(18)(19),
        list_of(26)(27)(17)(18),
        list_of(25)(26)(17)(18),
        list_of(25)(26)(16)(17),
        list_of(24)(25)(16)(17),
        list_of(25)(16)(17)(9),
        list_of(26)(17)(18)(10),
        list_of(17)(18)(10)(11),
        list_of(17)(18)(9)(10),
        list_of(16)(17)(9)(10),

        list_of(32)(33)(42)(43),
        list_of(23)(32)(33)(42),
        list_of(23)(24)(32)(33),
        list_of(23)(24)(33)(34),
        list_of(15)(23)(24)(33),
        list_of(15)(16)(23)(24),
        list_of(15)(16)(24)(25),
        list_of(8)(15)(16)(24),
        list_of(8)(9)(15)(16),
        list_of(8)(9)(16)(17);
 
    
    }

    return stations;
  }



  /*
   * Registration of geometries
   */

  struct GeometryPointers {
    GeometryPointers(const StationList* (*station3_)(),
		     const StationList* (*station4_)())
      : station3(station3_), station4(station4_)
    {}

    const StationList* (*station3)();
    const StationList* (*station4)();
  };

  typedef std::map<std::string, GeometryPointers> GeometryMap;

  const GeometryMap& geometryMap()
  {
    static const GeometryMap geometries =
      map_list_of("IC86", GeometryPointers(getStation3_IC86, getStation4_IC86))
                 ("IC79", GeometryPointers(getStation3_IC79, getStation4_IC79))
                 ("IC59", GeometryPointers(getStation3_IC59, getStation4_IC59))
                 ("IC40", GeometryPointers(getStation3_IC40, getStation4_IC40));

    return geometries;
  }

  const StationList* getStation3(const std::string &name)
  {
    GeometryMap::const_iterator it = geometryMap().find(name);
    if (it != geometryMap().end())
      return it->second.station3();
    else
      return 0;
  }

  const StationList* getStation4(const std::string &name)
  {
    GeometryMap::const_iterator it = geometryMap().find(name);
    if (it != geometryMap().end())
      return it->second.station4();
    else
      return 0;
  }

}
