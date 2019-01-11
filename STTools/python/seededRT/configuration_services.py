import math

from icecube.icetray import I3Units

from icecube.recclasses import OMKeySet, OMKeyLinkSet, I3VectorOMKeySet, I3VectorOMKeyLinkSet
from icecube.STTools.seededRT import I3SeededRTConfiguration
from icecube.STTools.seededRT import I3SeededRTConfigurationService

#===============================================================================
class I3DOMLinkSeededRTConfigurationService(I3SeededRTConfigurationService):
    """The I3DOMLinkSeededRTConfigurationService class provides a seededRT ST
    configuration service configured with DOM link ST configurations.

    Using this class, one can configure the seededRT algorithm with
    individual DOM-DOM-link ST configurations. For instance, two different ST
    configurations for IceCube and DeepCore DOMs. Additionally, a seperate ST
    configuration can be defined for IceCube-DeepCore DOM links. If it is not
    defined explicitly, the DeepCore-DeepCore DOM-link ST configuration will be
    used. Furthermore, DOM link ST configuration can be defined for DOM links
    including PINGU DOMs, i.e. IceCube-PINGU, DeepCore-PINGU, and PINGU-PINGU
    DOM links.
    """
    def __init__(self,
        allowSelfCoincidence         = False,
        useDustlayerCorrection       = True,
        dustlayerUpperZBoundary      = 0*I3Units.m,
        dustlayerLowerZBoundary      = -150*I3Units.m,
        ic_ic_RTRadius               = 150.*I3Units.m,
        ic_ic_RTTime                 = 1000.*I3Units.ns,
        ic_ic_RTCylinderHeight       = None,
        dc_dc_RTRadius               = None,
        dc_dc_RTTime                 = None,
        dc_dc_RTCylinderHeight       = None,
        pingu_pingu_RTRadius         = None,
        pingu_pingu_RTTime           = None,
        pingu_pingu_RTCylinderHeight = None,
        ic_dc_RTRadius               = None,
        ic_dc_RTTime                 = None,
        ic_dc_RTCylinderHeight       = None,
        ic_pingu_RTRadius            = None,
        ic_pingu_RTTime              = None,
        ic_pingu_RTCylinderHeight    = None,
        dc_pingu_RTRadius            = None,
        dc_pingu_RTTime              = None,
        dc_pingu_RTCylinderHeight    = None,
        ic_strings                   = None,
        ic_oms                       = None,
        dc_strings                   = None,
        dc_oms                       = None,
        pingu_strings                = None,
        pingu_oms                    = None,
        treat_string_36_as_deepcore  = True
    ):
        """Constructs a new I3SeededRTConfiguration object holding ST
        configurations individually for IceCube, DeepCore, and PINGU DOM links.

        :type  allowSelfCoincidence: bool
        :param allowSelfCoincidence: Switch if hits on the same OM can be in
            causial connection to each other (``True``) or not (``False``).

        :type  useDustlayerCorrection: bool
        :param useDustlayerCorrection: Switch to enable (``True``) or disable
            (``False``) the dust layer correction.

        :type dustlayerUpperZBoundary: float
        :param dustlayerUpperZBoundary: The upper bound of the z-coordinate of
            the dust layer. This is only used when the dust layer correction has
            been enabled.

        :type dustlayerLowerZBoundary: float
        :param dustlayerLowerZBoundary: The lower bound of the z-coordinate of
            the dust layer. This is only used when the dust layer correction has
            been enabled.

        :type  ic_ic_RTRadius: float
        :param ic_ic_RTRadius: The RT radius for IceCube-IceCube DOM links.

        :type  ic_ic_RTTime: float
        :param ic_ic_RTTime: The RT time for IceCube-IceCube DOM links.

        :type  ic_ic_RTCylinderHeight: float | None
        :param ic_ic_RTCylinderHeight: If set to a non-None value, the RT
            coordinate system for IceCube-IceCube DOM link RT hit condition
            calculations is to a cylindrical coordinate system instead of a
            spherical one. Then, this value specifies the height of the RT
            cylinder for IceCube-IceCube DOM links.

        :type  dc_dc_RTRadius: float | None
        :param dc_dc_RTRadius: The value of the RT radius for DeepCore-DeepCore
            DOM links. If ``dc_dc_RTTime`` is set to ``None``, this value will
            be set to the value of ``ic_ic_RTRadius``.

        :type  dc_dc_RTTime: float | None
        :param dc_dc_RTTime: The value of the RT time for DeepCore-DeepCore DOM
            links. If set to ``None``, it will be set to the value of
            ``ic_ic_RTTime``.

        :type  dc_dc_RTCylinderHeight: float | None
        :param dc_dc_RTCylinderHeight: If set to a non-None value, the RT
            coordinate system for DeepCore-DeepCore DOM link RT hit condition
            calculations is to a cylindrical coordinate system instead of a
            spherical one. Then, this value specifies the height of the RT
            cylinder for DeepCore-DeepCore DOM links.
            If ``dc_dc_RTTime`` is set to ``None``, this value will be set to
            the value of ``ic_ic_RTCylinderHeight``.

        :type  pingu_pingu_RTRadius: float | None
        :param pingu_pingu_RTRadius: The value of the RT radius for PINGU-PINGU
            DOM links. If ``pingu_pingu_RTTime`` is set to ``None``, this value
            will be set to the value of ``dc_dc_RTRadius``.

        :type  pingu_pingu_RTTime: float | None
        :param pingu_pingu_RTTime: The value of the RT time for PINGU-PINGU DOM
            links. If set to ``None``, it will be set to the value of
            ``dc_dc_RTTime``.

        :type  pingu_pingu_RTCylinderHeight: float | None
        :param pingu_pingu_RTCylinderHeight: If set to a non-None value, the RT
            coordinate system for PINGU-PINGU DOM link RT hit condition
            calculations is to a cylindrical coordinate system instead of a
            spherical one. Then, this value specifies the height of the RT
            cylinder for PINGU-PINGU DOM links.
            If ``pingu_pingu_RTTime`` is set to ``None``, this value will be set
            to the value of ``dc_dc_RTCylinderHeight``.

        :type  ic_dc_RTRadius: float | None
        :param ic_dc_RTRadius: The value of the RT radius for IceCube-DeepCore
            DOM links. If ``ic_dc_RTTime`` is set to ``None``, this value will
            be set to the value of ``dc_dc_RTRadius``.

        :type  ic_dc_RTTime: float | None
        :param ic_dc_RTTime: The value of the RT time for IceCube-DeepCore DOM
            links. If set to ``None``, it will be set to the value of
            ``dc_dc_RTTime``.

        :type  ic_dc_RTCylinderHeight: float | None
        :param ic_dc_RTCylinderHeight: If set to a non-None value, the RT
            coordinate system for IceCube-DeepCore DOM link RT hit condition
            calculations is to a cylindrical coordinate system instead of a
            spherical one. Then, this value specifies the height of the RT
            cylinder for IceCube-DeepCore DOM links.
            If ``ic_dc_RTTime`` is set to ``None``, this value will be set
            to the value of ``dc_dc_RTCylinderHeight``.

        :type  ic_pingu_RTRadius: float | None
        :param ic_pingu_RTRadius: The value of the RT radius for IceCube-PINGU
            DOM links. If ``ic_pingu_RTTime`` is set to ``None``, this value
            will be set to the value of ``pingu_pingu_RTRadius``.

        :type  ic_pingu_RTTime: float | None
        :param ic_pingu_RTTime: The value of the RT time for IceCube-PINGU DOM
            links. If set to ``None``, it will be set to the value of
            ``pingu_pingu_RTTime``.

        :type  ic_pingu_RTCylinderHeight: float | None
        :param ic_pingu_RTCylinderHeight: If set to a non-None value, the RT
            coordinate system for IceCube-PINGU DOM link RT hit condition
            calculations is to a cylindrical coordinate system instead of a
            spherical one. Then, this value specifies the height of the RT
            cylinder for IceCube-PINGU DOM links.
            If ``ic_pingu_RTTime`` is set to ``None``, this value will be set
            to the value of ``pingu_pingu_RTCylinderHeight``.

        :type  dc_pingu_RTRadius: float | None
        :param dc_pingu_RTRadius: The value of the RT radius for DeepCore-PINGU
            DOM links. If ``dc_pingu_RTTime`` is set to ``None``, this value
            will be set to the value of ``pingu_pingu_RTRadius``.

        :type  dc_pingu_RTTime: float | None
        :param dc_pingu_RTTime: The value of the RT time for DeepCore-PINGU DOM
            links. If set to ``None``, it will be set to the value of
            ``pingu_pingu_RTTime``.

        :type  dc_pingu_RTCylinderHeight: float | None
        :param dc_pingu_RTCylinderHeight: If set to a non-None value, the RT
            coordinate system for DeepCore-PINGU DOM link RT hit condition
            calculations is to a cylindrical coordinate system instead of a
            spherical one. Then, this value specifies the height of the RT
            cylinder for DeepCore-PINGU DOM links.
            If ``dc_pingu_RTTime`` is set to ``None``, this value will be set
            to the value of ``pingu_pingu_RTCylinderHeight``.

        :type  ic_strings: list of str | None
        :param ic_strings: The list of str objects defining the IceCube
            string numbers. Together with the ``ic_oms`` option, it defines the
            set of OMKeys for IceCube OMs.
            Each str element can contain a single number, e.g.
            ``"12"`` or a range of numbers, e.g. ``"2-6"``. If ``ic_strings``
            and ``ic_oms`` is set to ``None``, it will be set to ``["1-78"]``.
            But if ``treat_string_36_as_deepcore`` is set to ``True``, it will
            be set to ``["1-35", "36", "36", "37-78"]``.

            .. note::

                The ``ic_strings`` and ``ic_oms`` lists must be of the same
                length.

        :type  ic_oms: list of str | None
        :param ic_oms: The list of str objects defining the IceCube OM
            numbers. Together with the ``ic_strings`` option, it defines the
            set of OMKeys for IceCube OMs.
            Each str element can contain a single number, e.g.
            ``"12"`` or a range of numbers, e.g. ``"2-6"``. If ``ic_oms`` and
            ``ic_strings`` is set to ``None``, it will be set to ``["1-60"]``.
            But if ``treat_string_36_as_deepcore`` is set to ``True``, it will
            be set to ``["1-60", "1-19", "25-39", "1-60"]``.

            .. note::

                The ``ic_oms`` and ``ic_strings`` lists must be of the same
                length.

        :type  dc_strings: list of str | None
        :param dc_strings: The list of str objects defining the DeepCore
            string numbers. Together with the ``dc_oms`` option, it defines the
            set of OMKeys for DeepCore OMs.
            Each str element can contain a single number, e.g.
            ``"12"`` or a range of numbers, e.g. ``"2-6"``. If ``dc_strings``
            and ``dc_oms`` is set to ``None``, it will be set to
            ``["79-86"]``. But if ``treat_string_36_as_deepcore`` is set to
            ``True``, it will be set to ``["36", "36", "79-86"]``.

            .. note::

                The ``dc_strings`` and ``dc_oms`` lists must be of the same
                length.

        :type  dc_oms: list of str | None
        :param dc_oms: The list of str objects defining the DeepCore OM
            numbers. Together with the ``dc_strings`` option, it defines the
            set of OMKeys for DeepCore OMs.
            Each str element can contain a single number, e.g.
            ``"12"`` or a range of numbers, e.g. ``"2-6"``. If ``dc_oms``
            and ``dc_strings`` is set to ``None``, it will be set to
            ``["1-60"]``. But if ``treat_string_36_as_deepcore`` is set to
            ``True``, it will be set to ``["20-24", "40-60", "1-60"]``.

            .. note::

                The ``dc_oms`` and ``dc_strings`` lists must be of the same
                length.

        :type  pingu_strings: list of str | None
        :param pingu_strings: The list of str objects defining the PINGU
            string numbers. Together with the ``pingu_oms`` option, it defines
            the set of OMKeys for PINGU OMs.
            Each str element can contain a single number, e.g.
            ``"12"`` or a range of numbers, e.g. ``"2-6"``.
            If set to ``None``, no PINGU ST configuration will be set.

            .. note::

                The ``pingu_strings`` and ``pingu_oms`` lists must be of the
                same length.

        :type  pingu_oms: list of str | None
        :param pingu_oms: The list of str objects defining the PINGU OM
            numbers. Together with the ``pingu_strings`` option, it defines the
            set of OMKeys for PINGU OMs.
            Each str element can contain a single number, e.g.
            ``"12"`` or a range of numbers, e.g. ``"2-6"``.
            If set to ``None``, no PINGU ST configuration will be set.

            .. note::

                The ``pingu_oms`` and ``pingu_strings`` lists must be of the
                same length.

        :type  treat_string_36_as_deepcore: bool
        :param treat_string_36_as_deepcore: Switch, if string 36 should be
            treated as a DeepCore string in the configuration (``True``) or not
            ``False``. If set to ``True``, OMs 20-24 and 40-60 on string 36
            will be treated as DeepCore OMs.

        """
        I3SeededRTConfigurationService.__init__(self,
            allowSelfCoincidence    = allowSelfCoincidence,
            useDustlayerCorrection  = useDustlayerCorrection,
            dustlayerUpperZBoundary = dustlayerUpperZBoundary,
            dustlayerLowerZBoundary = dustlayerLowerZBoundary
        )

        #-----------------------------------------------------------------------
        # Generate the IC OMKeySet object defining all IC OMKeys.
        if(ic_strings is None and ic_oms is None):
            if(treat_string_36_as_deepcore):
                ic_strings = ["1-35", "36",   "36",    "37-78"]
                ic_oms     = ["1-60", "1-19", "25-39", "1-60"]
            else:
                ic_strings = ["1-78"]
                ic_oms     = ["1-60"]
        elif(ic_strings is None or ic_oms is None):
            raise ValueError("If ic_strings or ic_oms is specified, both must "
                             "be specified!")
        elif(len(ic_strings) != len(ic_oms)):
            raise ValueError("The lists of ic_strings and ic_oms must be of "
                             "equal length!")
        ic_OMKeySet = OMKeySet(ic_strings,ic_oms)

        # Generate the DC OMKeySet object defining all DC OMKeys.
        if(dc_strings is None and dc_oms is None):
            if(treat_string_36_as_deepcore):
                dc_strings = ["36",    "36",    "79-86"]
                dc_oms     = ["20-24", "40-60", "1-60"]
            else:
                dc_strings = ["79-86"]
                dc_oms     = ["1-60"]
        elif(dc_strings is None or dc_oms is None):
            raise ValueError("If dc_strings or dc_oms is specified, both must "
                             "be specified!")
        elif(len(dc_strings) != len(dc_oms)):
            raise ValueError("The lists of dc_strings and dc_oms must be of "
                             "equal length!")
        dc_OMKeySet = OMKeySet(dc_strings,dc_oms)

        # Generate the PINGU OMKeySet object defining all PINGU OMKeys.
        if(pingu_strings is None and pingu_oms is None):
            pingu_strings = []
            pingu_oms     = []
        elif(pingu_strings is None or pingu_oms is None):
            raise ValueError("If pingu_strings or pingu_oms is specified, both "
                             "must be specified!")
        elif(len(dc_strings) != len(dc_oms)):
            raise ValueError("The lists of pingu_strings and pingu_oms must be "
                             "of equal length!")
        pingu_OMKeySet = OMKeySet(pingu_strings,pingu_oms)

        #-----------------------------------------------------------------------
        # Determine all the (default) ST settings.
        if(ic_ic_RTCylinderHeight is not None):
            ic_ic_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Cyl
        else:
            ic_ic_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Sph
            ic_ic_RTCylinderHeight = float('nan')

        if(dc_dc_RTTime is None):
            dc_dc_RTTime           = ic_ic_RTTime
            dc_dc_RTRadius         = ic_ic_RTRadius
            dc_dc_RTCylinderHeight = ic_ic_RTCylinderHeight

        if(dc_dc_RTCylinderHeight is not None and not math.isnan(dc_dc_RTCylinderHeight)):
            dc_dc_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Cyl
        else:
            dc_dc_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Sph
            dc_dc_RTCylinderHeight = float('nan')

        if(pingu_pingu_RTTime is None):
            pingu_pingu_RTTime           = dc_dc_RTTime
            pingu_pingu_RTRadius         = dc_dc_RTRadius
            pingu_pingu_RTCylinderHeight = dc_dc_RTCylinderHeight

        if(pingu_pingu_RTCylinderHeight is not None and not math.isnan(pingu_pingu_RTCylinderHeight)):
            pingu_pingu_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Cyl
        else:
            pingu_pingu_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Sph
            pingu_pingu_RTCylinderHeight = float('nan')

        if(ic_dc_RTTime is None):
            ic_dc_RTTime           = dc_dc_RTTime
            ic_dc_RTRadius         = dc_dc_RTRadius
            ic_dc_RTCylinderHeight = dc_dc_RTCylinderHeight

        if(ic_dc_RTCylinderHeight is not None and not math.isnan(ic_dc_RTCylinderHeight)):
            ic_dc_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Cyl
        else:
            ic_dc_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Sph
            ic_dc_RTCylinderHeight = float('nan')

        if(ic_pingu_RTTime is None):
            ic_pingu_RTTime           = pingu_pingu_RTTime
            ic_pingu_RTRadius         = pingu_pingu_RTRadius
            ic_pingu_RTCylinderHeight = pingu_pingu_RTCylinderHeight

        if(ic_pingu_RTCylinderHeight is not None and not math.isnan(ic_pingu_RTCylinderHeight)):
            ic_pingu_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Cyl
        else:
            ic_pingu_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Sph
            ic_pingu_RTCylinderHeight = float('nan')

        if(dc_pingu_RTTime is None):
            dc_pingu_RTTime           = pingu_pingu_RTTime
            dc_pingu_RTRadius         = pingu_pingu_RTRadius
            dc_pingu_RTCylinderHeight = pingu_pingu_RTCylinderHeight

        if(dc_pingu_RTCylinderHeight is not None and not math.isnan(dc_pingu_RTCylinderHeight)):
            dc_pingu_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Cyl
        else:
            dc_pingu_RTCoordSys = I3SeededRTConfiguration.SeededRTCoordSys.Sph
            dc_pingu_RTCylinderHeight = float('nan')

        #-----------------------------------------------------------------------
        # Now we create ST DOM link configurations for the sub-detectors.

        # ... for the IceTop OMs.
        # The IceTop OMs are needed here, because if they are not present in the
        # ST configuration but are present in the I3Geometry, the completeness
        # check of the ST configuration service will fail.
        # Note: The completeness check requires only, that each DOM is present
        #       in at least one DOM link. So we do not need to create all IceTop
        #       link combinations.
        it_OMKeySet = OMKeySet(["0", "1-11",  "12",    "13-61", "62",    "63-86"], # OMs 65 and 66 for string 12 and 62 are required for scintillators.
                               ["1", "61-64", "61-66", "61-64", "61-66", "61-64"]) # OM 0, 1 is IceAct
        it_it_OMKeyLinkSets = I3VectorOMKeyLinkSet([OMKeyLinkSet(it_OMKeySet, it_OMKeySet)])
        self.st_config_vec.append(I3SeededRTConfiguration(
            name          = "IT-IT",
            omKeyLinkSets = it_it_OMKeyLinkSets,
            rtCoordSys    = I3SeededRTConfiguration.SeededRTCoordSys.Sph,
            rtTime        = 0,
            rtRadius      = 0,
            rtHeight      = float('nan')
        ))

        # ... for the IceCube - IceCube OM links.
        ic_ic_OMKeyLinkSets = I3VectorOMKeyLinkSet([OMKeyLinkSet(ic_OMKeySet, ic_OMKeySet)])
        self.st_config_vec.append(I3SeededRTConfiguration(
            name          = "IC-IC",
            omKeyLinkSets = ic_ic_OMKeyLinkSets,
            rtCoordSys    = ic_ic_RTCoordSys,
            rtTime        = ic_ic_RTTime,
            rtRadius      = ic_ic_RTRadius,
            rtHeight      = ic_ic_RTCylinderHeight
        ))

        # ... for the DeepCore - DeepCore OM links.
        dc_dc_OMKeyLinkSets = I3VectorOMKeyLinkSet([OMKeyLinkSet(dc_OMKeySet, dc_OMKeySet)])
        self.st_config_vec.append(I3SeededRTConfiguration(
            name          = "DC-DC",
            omKeyLinkSets = dc_dc_OMKeyLinkSets,
            rtCoordSys    = dc_dc_RTCoordSys,
            rtTime        = dc_dc_RTTime,
            rtRadius      = dc_dc_RTRadius,
            rtHeight      = dc_dc_RTCylinderHeight
        ))

        # ... for IceCube - DeepCore OM links.
        ic_dc_OMKeyLinkSets = I3VectorOMKeyLinkSet([OMKeyLinkSet(ic_OMKeySet, dc_OMKeySet)])
        self.st_config_vec.append(I3SeededRTConfiguration(
            name          = "IC-DC",
            omKeyLinkSets = ic_dc_OMKeyLinkSets,
            rtCoordSys    = ic_dc_RTCoordSys,
            rtTime        = ic_dc_RTTime,
            rtRadius      = ic_dc_RTRadius,
            rtHeight      = ic_dc_RTCylinderHeight
        ))

        # If PINGU OMKeys were specified.
        # At the time this code was written, the PINGU geometry was not
        # fixed, so we keep the set of PINGU OMs as a parameter to the
        # user.
        if(not pingu_OMKeySet.is_empty):
            # ... for the PINGU - PINGU OM links.
            pingu_pingu_OMKeyLinkSets = I3VectorOMKeyLinkSet([OMKeyLinkSet(pingu_OMKeySet, pingu_OMKeySet)])
            self.st_config_vec.append(I3SeededRTConfiguration(
                name          = "PINGU-PINGU",
                omKeyLinkSets = pingu_pingu_OMKeyLinkSets,
                rtCoordSys    = pingu_pingu_RTCoordSys,
                rtTime        = pingu_pingu_RTTime,
                rtRadius      = pingu_pingu_RTRadius,
                rtHeight      = pingu_pingu_RTCylinderHeight
            ))

            # ... for IceCube - PINGU OM links.
            ic_pingu_OMKeyLinkSets = I3VectorOMKeyLinkSet([OMKeyLinkSet(ic_OMKeySet, pingu_OMKeySet)])
            self.st_config_vec.append(I3SeededRTConfiguration(
                name          = "IC-PINGU",
                omKeyLinkSets = ic_pingu_OMKeyLinkSets,
                rtCoordSys    = ic_pingu_RTCoordSys,
                rtTime        = ic_pingu_RTTime,
                rtRadius      = ic_pingu_RTRadius,
                rtHeight      = ic_pingu_RTCylinderHeight
            ))

            # ... for DeepCore - PINGU OM links.
            dc_pingu_OMKeyLinkSets = I3VectorOMKeyLinkSet([OMKeyLinkSet(dc_OMKeySet, pingu_OMKeySet)])
            self.st_config_vec.append(I3SeededRTConfiguration(
                name          = "DC-PINGU",
                omKeyLinkSets = dc_pingu_OMKeyLinkSets,
                rtCoordSys    = dc_pingu_RTCoordSys,
                rtTime        = dc_pingu_RTTime,
                rtRadius      = dc_pingu_RTRadius,
                rtHeight      = dc_pingu_RTCylinderHeight
            ))

        # Check if the global ST configuration settings are consistant.
        self.AreGlobalSettingsConsistant(throwException=True)

        # Freeze the ST configuration for its fast access.
        self.FreezeSTConfiguration()
