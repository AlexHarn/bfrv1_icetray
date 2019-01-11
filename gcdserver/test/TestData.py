
SNOW_DEPTH_XML = """<?xml version="1.0" encoding="UTF-8"?>
<IceTopStationTankSnow>
  <Date> 2011-2-1 00:00:00 </Date>
  <Tank>
    <!-- This is Tank 1 A  -->
    <StringId> 1 </StringId>
    <TankLabel> A </TankLabel>
    <SnowHeight> 0.077 </SnowHeight>
  </Tank>
</IceTopStationTankSnow>
"""

BASELINE_XML = """<?xml version="1.0" ?>
<baselines>
    <date>2016-03-18</date>
    <time>16:33:41</time>
    <dom StringId="1" TubeId="61">
        <ATWDChipAChan0>125.068580</ATWDChipAChan0>
        <ATWDChipAChan1>136.172671</ATWDChipAChan1>
        <ATWDChipAChan2>136.172799</ATWDChipAChan2>
        <ATWDChipBChan0>129.730441</ATWDChipBChan0>
        <ATWDChipBChan1>134.035253</ATWDChipBChan1>
        <ATWDChipBChan2>136.943712</ATWDChipBChan2>
        <FADC>137.185200</FADC>
    </dom>
    <dom StringId="1" TubeId="62">
        <ATWDChipAChan0>126.925628</ATWDChipAChan0>
        <ATWDChipAChan1>131.162387</ATWDChipAChan1>
        <ATWDChipAChan2>132.048000</ATWDChipAChan2>
        <ATWDChipBChan0>129.852887</ATWDChipBChan0>
        <ATWDChipBChan1>136.098590</ATWDChipBChan1>
        <ATWDChipBChan2>138.041781</ATWDChipBChan2>
        <FADC>126.028301</FADC>
    </dom>
</baselines>
"""

VEMCAL_XML = """<?xml version="1.0" encoding="UTF-8"?>
<VEMCalibOm>
  <Date> 2014-01-01 00:39:07 </Date>
  <FirstRun> 123614 </FirstRun>
  <LastRun> 123968 </LastRun>
  <DOM>
    <!-- This is DOM 1-61  -->
    <StringId> 1 </StringId>
    <TubeId> 61 </TubeId>
    <pePerVEM> 116.274 </pePerVEM>
    <muPeakWidth> 20.3121 </muPeakWidth>
    <sigBkgRatio> 10.306 </sigBkgRatio>
    <corrFactor> 1 </corrFactor>
    <hglgCrossOver> 2361.66 </hglgCrossOver>
    <muonFitStatus> 0 </muonFitStatus>
    <muonFitRchi2> 1.42718 </muonFitRchi2>
    <hglgFitStatus> 0 </hglgFitStatus>
    <hglgFitRchi2> 2.44815 </hglgFitRchi2>
  </DOM>
  <DOM>
    <!-- This is DOM 1-62  -->
    <StringId> 1 </StringId>
    <TubeId> 62 </TubeId>
    <pePerVEM> 134.187 </pePerVEM>
    <muPeakWidth> 20.3121 </muPeakWidth>
    <sigBkgRatio> 10.306 </sigBkgRatio>
    <corrFactor> 1 </corrFactor>
    <hglgCrossOver> -1 </hglgCrossOver>
  </DOM>
</VEMCalibOm>
"""

NOISE_RATE_JSON = """
{ 
    "01,61": {
        "rate_hz": 1386.3499999999999, 
        "stddev_hz": 11.829628136113188
    },
    "01,62": {
        "rate_hz": 54.650000000000006, 
        "stddev_hz": 2.1974975932091501
    }
}
"""

SPE_FIT_JSON = """
{
    "1,61" : 
    {
        "ATWD_fit" : 
        {
            "chi2" : 467.02450517427076,
            "error" : 0.058159380752974804,
            "exp_norm" : 6220.3061751023652,
            "exp_scale" : 0.33863255865923259,
            "gaus_mean" : 1.0341419190237735,
            "gaus_norm" : 7206.079169751908,
            "gaus_stddev" : 0.25851599249418755,
            "ndf" : 80,
            "nentries" : 749066,
            "valid" : true,
            "x_max_f" : 1.0375000000000001
        },
        "FADC_fit" : 
        {
            "chi2" : 1519.5616360369083,
            "error" : 1.0566133983913948,
            "exp_norm" : 7689.7387282543104,
            "exp_scale" : 0.3881778140705533,
            "gaus_mean" : 1.1160816676546705,
            "gaus_norm" : 5625.7635408351916,
            "gaus_stddev" : 0.23783334934491418,
            "ndf" : 71,
            "nentries" : 700676,
            "valid" : true,
            "x_max_f" : 1.0125
        },
        "JOINT_fit" : 
        {
            "chi2" : 549.80212439107402,
            "error" : 0.057953681420883474,
            "exp_norm" : 5630.9241121652794,
            "exp_scale" : 0.25054968276866763,
            "gaus_mean" : 1.0325212232516821,
            "gaus_norm" : 6497.3366978454396,
            "gaus_stddev" : 0.25579619731120745,
            "ndf" : 80,
            "nentries" : 740434,
            "valid" : true,
            "x_max_f" : 1.0375000000000001
        }
    },
    "1,62" : 
    {
        "ATWD_fit" : 
        {
            "chi2" : 375.48756348297974,
            "error" : 0.021099427725282678,
            "exp_norm" : 7307.8142118371961,
            "exp_scale" : 0.27319987688009584,
            "gaus_mean" : 1.0544911706904694,
            "gaus_norm" : 9110.3852070359335,
            "gaus_stddev" : 0.24610190517273389,
            "ndf" : 79,
            "nentries" : 964340,
            "valid" : true,
            "x_max_f" : 1.0625
        },
        "FADC_fit" : 
        {
            "chi2" : 4335.2397126699916,
            "error" : 3.1526110454795639,
            "exp_norm" : 7773.8095936775953,
            "exp_scale" : 0.26444398625209198,
            "gaus_mean" : 1.0715882684510842,
            "gaus_norm" : 8012.8661928377996,
            "gaus_stddev" : 0.22621182994260097,
            "ndf" : 72,
            "nentries" : 909231,
            "valid" : true,
            "x_max_f" : 1.1375
        },
        "JOINT_fit" : 
        {
            "chi2" : 467.81781249779885,
            "error" : 0.022952108002293914,
            "exp_norm" : 6728.183586839823,
            "exp_scale" : 0.19590329312561494,
            "gaus_mean" : 1.0543760019040789,
            "gaus_norm" : 8260.8984581176519,
            "gaus_stddev" : 0.24293675094300488,
            "ndf" : 79,
            "nentries" : 954274,
            "valid" : true,
            "x_max_f" : 1.0625
        }
    }
}
"""

DOMCAL_XML_1_61 = """<?xml version="1.0" encoding="UTF-8"?>
<domcal version="7.6.0">
  <date>4-3-2015</date>
  <time>09:19:37</time>
  <domid>a2a7871499f0</domid>
  <temperature format="Kelvin">244.5</temperature>
  <frontEndImpedance format="Ohms">50.0</frontEndImpedance>
  <discriminator id="spe">
    <fit model="linear">
      <param name="slope">0.012365</param>
      <param name="intercept">-6.69295</param>
      <regression-coeff>0.999953</regression-coeff>
    </fit>
  </discriminator>
  <discriminator id="mpe">
    <fit model="linear">
      <param name="slope">0.126135</param>
      <param name="intercept">-67.6154</param>
      <regression-coeff>0.999952</regression-coeff>
    </fit>
  </discriminator>
  <atwd id="0" channel="2" bin="5">
    <fit model="linear">
      <param name="slope">-0.00195929</param>
      <param name="intercept">2.85138</param>
      <regression-coeff>0.999997</regression-coeff>
    </fit>
  </atwd>
  <fadc_baseline>
    <fit model="linear">
      <param name="slope">1.26364</param>
      <param name="intercept">-886.734</param>
      <regression-coeff>0.999996</regression-coeff>
    </fit>
  </fadc_baseline>
  <fadc_gain>
    <gain error="2.86272e-08">8.79808e-05</gain>
  </fadc_gain>
  <fadc_delta_t>
    <delta_t error="0.146785">-113.479</delta_t>
  </fadc_delta_t>
  <atwd_delta_t id="0">
    <delta_t error="0">0</delta_t>
  </atwd_delta_t>
  <atwd_delta_t id="1">
    <delta_t error="0.00439028">-0.602172</delta_t>
  </atwd_delta_t>
  <amplifier channel="0">
    <gain error="0.00317694">-15.8162</gain>
  </amplifier>
  <amplifier channel="1">
    <gain error="0.000166218">-1.80474</gain>
  </amplifier>
  <amplifier channel="2">
    <gain error="4.94922e-06">-0.210016</gain>
  </amplifier>
  <atwdfreq atwd="0">
    <fit model="quadratic">
      <param name="c0">36.3884</param>
      <param name="c1">0.349686</param>
      <param name="c2">-3.01701e-05</param>
      <regression-coeff>0.999889</regression-coeff>
    </fit>
  </atwdfreq>
  <atwdfreq atwd="1">
    <fit model="quadratic">
      <param name="c0">37.9335</param>
      <param name="c1">0.35192</param>
      <param name="c2">-2.98137e-05</param>
      <regression-coeff>0.999851</regression-coeff>
    </fit>
  </atwdfreq>
  <baseline voltage="0">
    <base atwd="0" channel="0" value="-3.27385e-05"/>
    <base atwd="0" channel="1" value="0.000466693"/>
    <base atwd="0" channel="2" value="0.000114595"/>
    <base atwd="1" channel="0" value="-0.000583816"/>
    <base atwd="1" channel="1" value="2.26812e-05"/>
    <base atwd="1" channel="2" value="6.24776e-05"/>
  </baseline>
  <pmtTransitTime num_pts="10">
    <fit model="linear">
      <param name="slope">1978.53</param>
      <param name="intercept">88.4719</param>
      <regression-coeff>0.998053</regression-coeff>
    </fit>
  </pmtTransitTime>
  <hvGainCal>
    <fit model="linear">
      <param name="slope">7.54566</param>
      <param name="intercept">-16.4696</param>
      <regression-coeff>0.999899</regression-coeff>
    </fit>
  </hvGainCal>
  <pmtDiscCal num_pts="8">
    <fit model="linear">
      <param name="slope">0.0155336</param>
      <param name="intercept">-8.34602</param>
      <regression-coeff>0.999263</regression-coeff>
    </fit>
  </pmtDiscCal>
</domcal>
"""

DOMCAL_XML_1_62 = """
<domcal version="7.6.0">
  <date>4-3-2015</date>
  <time>09:19:37</time>
  <domid>19424ffec9a4</domid>
  <temperature format="Kelvin">244.5</temperature>
  <frontEndImpedance format="Ohms">50.0</frontEndImpedance>
  <discriminator id="spe">
    <fit model="linear">
      <param name="slope">0.012365</param>
      <param name="intercept">-6.69295</param>
      <regression-coeff>0.999953</regression-coeff>
    </fit>
  </discriminator>
  <discriminator id="mpe">
    <fit model="linear">
      <param name="slope">0.126135</param>
      <param name="intercept">-67.6154</param>
      <regression-coeff>0.999952</regression-coeff>
    </fit>
  </discriminator>
  <atwd id="0" channel="2" bin="5">
    <fit model="linear">
      <param name="slope">-0.00195929</param>
      <param name="intercept">2.85138</param>
      <regression-coeff>0.999997</regression-coeff>
    </fit>
  </atwd>
  <fadc_baseline>
    <fit model="linear">
      <param name="slope">1.26364</param>
      <param name="intercept">-886.734</param>
      <regression-coeff>0.999996</regression-coeff>
    </fit>
  </fadc_baseline>
  <fadc_gain>
    <gain error="2.86272e-08">8.79808e-05</gain>
  </fadc_gain>
  <fadc_delta_t>
    <delta_t error="0.146785">-113.479</delta_t>
  </fadc_delta_t>
  <atwd_delta_t id="0">
    <delta_t error="0">0</delta_t>
  </atwd_delta_t>
  <atwd_delta_t id="1">
    <delta_t error="0.00439028">-0.602172</delta_t>
  </atwd_delta_t>
  <amplifier channel="0">
    <gain error="0.00317694">-15.8162</gain>
  </amplifier>
  <amplifier channel="1">
    <gain error="0.000166218">-1.80474</gain>
  </amplifier>
  <amplifier channel="2">
    <gain error="4.94922e-06">-0.210016</gain>
  </amplifier>
  <atwdfreq atwd="0">
    <fit model="quadratic">
      <param name="c0">36.3884</param>
      <param name="c1">0.349686</param>
      <param name="c2">-3.01701e-05</param>
      <regression-coeff>0.999889</regression-coeff>
    </fit>
  </atwdfreq>
  <atwdfreq atwd="1">
    <fit model="quadratic">
      <param name="c0">37.9335</param>
      <param name="c1">0.35192</param>
      <param name="c2">-2.98137e-05</param>
      <regression-coeff>0.999851</regression-coeff>
    </fit>
  </atwdfreq>
  <baseline voltage="0">
    <base atwd="0" channel="0" value="-3.27385e-05"/>
    <base atwd="0" channel="1" value="0.000466693"/>
    <base atwd="0" channel="2" value="0.000114595"/>
    <base atwd="1" channel="0" value="-0.000583816"/>
    <base atwd="1" channel="1" value="2.26812e-05"/>
    <base atwd="1" channel="2" value="6.24776e-05"/>
  </baseline>
  <pmtTransitTime num_pts="10">
    <fit model="linear">
      <param name="slope">1978.53</param>
      <param name="intercept">88.4719</param>
      <regression-coeff>0.998053</regression-coeff>
    </fit>
  </pmtTransitTime>
  <hvGainCal>
    <fit model="linear">
      <param name="slope">7.54566</param>
      <param name="intercept">-16.4696</param>
      <regression-coeff>0.999899</regression-coeff>
    </fit>
  </hvGainCal>
  <pmtDiscCal num_pts="8">
    <fit model="linear">
      <param name="slope">0.0155336</param>
      <param name="intercept">-8.34602</param>
      <regression-coeff>0.999263</regression-coeff>
    </fit>
  </pmtDiscCal>
</domcal>
"""


DOM_DROOP_1_61 = {'objectName': "AP8P3015",
                  'objectType': 'Toroid Droop',
                  'data': {'ATWD Sigma': 0.35,
                           'FADC Sigma': 0.42,
                           'tau0': 222.97,
                           'tau1': 4267.6001,
                           'tau2': 21.552,
                           'tau3': 167.227,
                           'tau4': 3200.7,
                           'tau5': 21.552,
                           'tauFraction': -3.3}
                  }


GEOMETRY_1_61 = {"objectName" : "AP8P3015",
                 "objectType" : "IceTop DOM",
                 "data": {"tank" : "A",
                          "string" : 1,
                          "position" : 61,
                          "toroidType" : "New",
                          "pmtType" : "R7081",
                          "mbid" : "a2a7871499f0",
                          "x" : -265.58,
                          "y" : -497.61,
                          "z" : 1944.7,
                          "nickname" : "Rome_1960",
                          "orientation" : "Down"}
                 }


GEOMETRY_1_62 = {"objectName" : "AP7P2042",
                 "objectType" : "IceTop DOM",
                 "data" : {"tank" : "A",
                           "string" : 1,
                           "position" : 62,
                           "toroidType" : "New",
                           "pmtType" : "R7081-100",
                           "mbid" : "19424ffec9a4",
                           "x" : -265.48,
                           "y" : -498.18,
                           "z" : 1944.7,
                           "nickname" : "Mount_Noshaq",
                           "orientation" : "Down"}
                 }


GEOMETRY_12_65 = {"objectName" : "SCIN0001",
                  "objectType" : "IceCube Scintillator",
                  "data" : {"sensorArea" : 1.7475,
                            "string" : 12,
                            "position" : 65,
                            "toroidType" : "New",
                            "mbid" : "0f8bea7ac17e",
                            "x" : 279.23,
                            "y" : -300.53,
                            "z" : 1946.2,
                            "nickname" : "Pizza_Margherita",
                            "orientation" : "Up"}
                  }


GEOMETRY_TANK_1A = {"objectName" : "Tank1A",
                    "objectType" : "IceTop Tank",
                    "data" : {"tankLiner" : "Zirconium",
                              "tank" : "A",
                              "string" : 1 }
                   }


def getRunConfigXML(trigConfName, domConfName):
    
    return """<?xml version='1.0' encoding='ASCII'?>
        <runConfig>
          <stringHub hubId="1" domConfig="%s"/>
          <triggerConfig>%s</triggerConfig>
          <runComponent name="inIceTrigger"/>
          <runComponent name="iceTopTrigger"/>
          <runComponent name="globalTrigger"/>
          <runComponent name="eventBuilder"/>
          <runComponent name="secondaryBuilders"/>
        </runConfig>""" % (domConfName, trigConfName)


DOM_CONFIG_XML = """<?xml version='1.0' encoding='ASCII'?>
<domConfigList>
    <domConfig mbid="a2a7871499f0" name="Rome_1960">
        <format> <deltaCompressed/> </format>
        <triggerMode> mpe </triggerMode>
        <atwd0TriggerBias>829</atwd0TriggerBias>
        <atwd1TriggerBias>802</atwd1TriggerBias>
        <atwd0RampRate> 350 </atwd0RampRate>
        <atwd1RampRate> 350 </atwd1RampRate>
        <atwd0RampTop> 2300 </atwd0RampTop>
        <atwd1RampTop> 2300 </atwd1RampTop>
        <atwdAnalogRef> 2250 </atwdAnalogRef>
        <frontEndPedestal> 2130 </frontEndPedestal>
        <fastAdcRef> 800 </fastAdcRef>
        <speTriggerDiscriminator>600</speTriggerDiscriminator>
        <mpeTriggerDiscriminator>575</mpeTriggerDiscriminator>
        <internalPulser> 0 </internalPulser>
        <ledBrightness> 1023 </ledBrightness>
        <frontEndAmpLowerClamp> 0 </frontEndAmpLowerClamp>
        <flasherRef> 450 </flasherRef>
        <muxBias> 500 </muxBias>
     <pmtHighVoltage>2523</pmtHighVoltage>
        <analogMux> off </analogMux>
        <pulserMode> beacon </pulserMode>
        <pulserRate> 5 </pulserRate>
        <hardwareMonitorInterval> 1 </hardwareMonitorInterval>
     <pedestalSettings>
         <averagePedestal atwd="A" ch="0"> 125</averagePedestal>
         <averagePedestal atwd="A" ch="1"> 136</averagePedestal>
         <averagePedestal atwd="A" ch="2"> 136</averagePedestal>
         <averagePedestal atwd="B" ch="0"> 130</averagePedestal>
         <averagePedestal atwd="B" ch="1"> 134</averagePedestal>
         <averagePedestal atwd="B" ch="2"> 137</averagePedestal>
     </pedestalSettings>
        <pedestalSubtract> true </pedestalSubtract>
        <chargeStamp type="atwd"/>
        <localCoincidence>
            <type> soft </type>
            <mode> up-or-down </mode>
            <txMode> both </txMode>
            <source> mpe </source>
            <span> 1 </span>
            <preTrigger> 1000 </preTrigger>
            <postTrigger> 2000 </postTrigger>
            <cableLength dir="down" dist="1"> 650 </cableLength>
            <cableLength dir="up" dist="1"> 650 </cableLength>
            <cableLength dir="down" dist="2"> 1350 </cableLength>
            <cableLength dir="up" dist="2"> 1350 </cableLength>
        </localCoincidence>
        <supernovaMode enabled="true">
            <deadtime> 250000 </deadtime>
            <disc> spe </disc>
        </supernovaMode>
        <scalerDeadtime> 51200 </scalerDeadtime>
        <enableIceTopMinBias/>
    </domConfig>
<domConfig mbid="19424ffec9a4" name="Mount_Noshaq">
        <format> <deltaCompressed/> </format>
        <triggerMode> spe </triggerMode>
        <atwd0TriggerBias>814</atwd0TriggerBias>
        <atwd1TriggerBias>819</atwd1TriggerBias>
        <atwd0RampRate> 350 </atwd0RampRate>
        <atwd1RampRate> 350 </atwd1RampRate>
        <atwd0RampTop> 2300 </atwd0RampTop>
        <atwd1RampTop> 2300 </atwd1RampTop>
        <atwdAnalogRef> 2250 </atwdAnalogRef>
        <frontEndPedestal> 2130 </frontEndPedestal>
        <fastAdcRef> 800 </fastAdcRef>
        <speTriggerDiscriminator>620</speTriggerDiscriminator>
        <mpeTriggerDiscriminator>550</mpeTriggerDiscriminator>
        <internalPulser> 0 </internalPulser>
        <ledBrightness> 1023 </ledBrightness>
        <frontEndAmpLowerClamp> 0 </frontEndAmpLowerClamp>
        <flasherRef> 450 </flasherRef>
        <muxBias> 500 </muxBias>
     <pmtHighVoltage>1401</pmtHighVoltage>
        <analogMux> off </analogMux>
        <pulserMode> beacon </pulserMode>
        <pulserRate> 5 </pulserRate>
        <hardwareMonitorInterval> 1 </hardwareMonitorInterval>
     <pedestalSettings>
         <averagePedestal atwd="A" ch="0"> 127</averagePedestal>
         <averagePedestal atwd="A" ch="1"> 131</averagePedestal>
         <averagePedestal atwd="A" ch="2"> 132</averagePedestal>
         <averagePedestal atwd="B" ch="0"> 130</averagePedestal>
         <averagePedestal atwd="B" ch="1"> 136</averagePedestal>
         <averagePedestal atwd="B" ch="2"> 138</averagePedestal>
     </pedestalSettings>
        <pedestalSubtract> true </pedestalSubtract>
        <chargeStamp type="atwd"/>
        <localCoincidence>
            <type> soft </type>
            <mode> up-or-down </mode>
            <txMode> none </txMode>
            <source> spe </source>
            <span> 1 </span>
            <preTrigger> 1000 </preTrigger>
            <postTrigger> 2000 </postTrigger>
            <cableLength dir="down" dist="1"> 650 </cableLength>
            <cableLength dir="up" dist="1"> 650 </cableLength>
            <cableLength dir="down" dist="2"> 1350 </cableLength>
            <cableLength dir="up" dist="2"> 1350 </cableLength>
        </localCoincidence>
        <supernovaMode enabled="true">
            <deadtime> 250000 </deadtime>
            <disc> spe </disc>
        </supernovaMode>
        <scalerDeadtime> 51200 </scalerDeadtime>
    </domConfig>
</domConfigList>
"""

TRIGGER_CONF_XML = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<activeTriggers configurationId="1610">

    <triggerConfig>
        <triggerType>3</triggerType>
        <triggerConfigId>-1</triggerConfigId>
        <sourceId>6000</sourceId>
        <triggerName>ThroughputTrigger</triggerName>
    </triggerConfig>
      
    <triggerConfig>
        <triggerType>23</triggerType>
        <triggerConfigId>23050</triggerConfigId>
        <sourceId>4000</sourceId>
        <triggerName>FixedRateTrigger</triggerName>
        <parameterConfig>
            <parameterName>interval</parameterName>
            <parameterValue>300000000000</parameterValue>
        </parameterConfig>
        <readoutConfig>
            <readoutType>0</readoutType>
            <timeOffset>0</timeOffset>
            <timeMinus>5000000</timeMinus>
            <timePlus>5000000</timePlus>
        </readoutConfig>
    </triggerConfig>

    <triggerConfig>
        <triggerType>0</triggerType>
        <triggerConfigId>1006</triggerConfigId>
        <sourceId>4000</sourceId>
        <triggerName>SimpleMajorityTrigger</triggerName>
        <parameterConfig>
            <parameterName>threshold</parameterName>
            <parameterValue>8</parameterValue>
        </parameterConfig>
        <parameterConfig>
            <parameterName>timeWindow</parameterName>
            <parameterValue>5000</parameterValue>
        </parameterConfig>
        <readoutConfig>
            <readoutType>1</readoutType>
            <timeOffset>0</timeOffset>
            <timeMinus>10000</timeMinus>
            <timePlus>10000</timePlus>
        </readoutConfig>
        <readoutConfig>
            <readoutType>2</readoutType>
            <timeOffset>0</timeOffset>
            <timeMinus>4000</timeMinus>
            <timePlus>6000</timePlus>
        </readoutConfig>
    </triggerConfig>
</activeTriggers>
"""