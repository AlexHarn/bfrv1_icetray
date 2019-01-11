#
# $Id$
#

i3_project(VHESelfVeto
  PYTHON_DIR python
  DOCS_DIR resources/docs)

i3_add_library(VHESelfVeto
  private/VHESelfVeto/DetectorShrinker.cxx
  private/VHESelfVeto/BadStringRemover.cxx
  private/VHESelfVeto/VHESelfVeto.cxx
  private/VHESelfVeto/VHESelfVetoUtils.cxx
  private/VHESelfVeto/HomogenizedQTot.cxx
  private/VHESelfVeto/LayerVeto.cxx
  private/VHESelfVeto/VertexInFiducialVolume.cxx
  private/VHESelfVeto/TauGeneratesMuon.cxx
  private/VHESelfVeto/FiducialVolumeEntryPointFinder.cxx
  private/clipper/clipper.cxx
  USE_PROJECTS serialization icetray dataclasses recclasses
  USE_TOOLS boost python
)

i3_add_pybindings(VHESelfVeto
  private/pybindings/module.cxx
  USE_TOOLS boost python
  USE_PROJECTS icetray dataclasses recclasses VHESelfVeto
  )
