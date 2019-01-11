#
# $Id$
#
i3_project(ipdf
  PYTHON_DIR python
  DOCS_DIR resources/docs
)

i3_add_library(ipdf 
  private/Hypotheses/DirectionalCascade.cxx
  private/Hypotheses/InfiniteMuon.cxx
  private/Hypotheses/PointCascade.cxx
  private/I3/I3DetectorConfiguration.cxx
  private/I3/I3DetectorResponse.cxx
  private/I3/I3HitOm.cxx
  private/I3/I3OmReceiver.cxx
  private/I3/I3PEHit.cxx
  private/I3/I3MediumPropertiesFile.cxx
  private/I3/IceTableInterpolator.cxx
  private/Pandel/BoxConvolutedPEP.cxx
  private/Pandel/ConvolutedHyperg.cxx
  private/Pandel/GConvolutePEP.cxx
  private/Pandel/GSemiConvolutePEP.cxx
  private/Pandel/GaussConvolutedPEP.cxx
  private/Pandel/IceModel.cxx
  private/Pandel/UPatchPEP.cxx
  private/Pandel/UnConvolutedPEP.cxx
  private/ipdf/PdfMath.cxx 
  private/ipdf/SimplePEHit.cxx
  private/PhotoSpline/PhotoSplinePEP.cxx

  USE_TOOLS gsl boost python

  USE_PROJECTS serialization icetray dataclasses phys-services photonics-service

  NOT_INSPECTABLE
)

i3_add_pybindings(ipdf
    private/pybindings/PEP.cxx
    private/pybindings/Likelihood.cxx
    private/pybindings/Ice.cxx
    private/pybindings/module.cxx

    USE_TOOLS boost python
    USE_PROJECTS icetray dataclasses ipdf
)

i3_executable(tutorial 
  private/tutorial/*.cxx
  USE_TOOLS gsl
  USE_PROJECTS ipdf
)

i3_test_executable(test 
  private/test/AllOMsLikelihoodTest.cxx
  private/test/DirectionalCascadeTest.cxx
  private/test/GaussConvolutedPrecisionTest.cxx
  private/test/I3OmReceiverTest.cxx
  private/test/I3PEPTest.cxx
  private/test/InfiniteMuonTest.cxx
  private/test/IPdfExceptionTest.cxx
  private/test/LayeredIceTest.cxx
  private/test/LikelihoodTest.cxx
  private/test/main.cxx
  private/test/MPETest.cxx
  private/test/PandelPEPTest.cxx
  private/test/PointCascadeTest.cxx
  private/test/SimpleOmReceiverTest.cxx
  private/test/SimplePEPTest.cxx

  USE_PROJECTS ipdf
)


i3_test_scripts(resources/scripts/*.py)
