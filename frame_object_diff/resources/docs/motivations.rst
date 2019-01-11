Motivation
==========

Claudio wants to send GCD information from the South Pole to the North
in real time, which has a message size limit of 50KB. We can take advantage
of the fact that the GCD almost never changes to only send the parts that
**do** change. Thus, this project was born.

Why a New Project?
------------------

From a discussion on slack (2015-03-10)::

  dschultz: olivas: as the "coordinator", what are your thoughts on where to put claudiok's Diff classes and modules?
          directly in dataclasses, or in a separate project?
  
  claudiok: you know my opinion
          these shouldn't go in dataclasses if you ask me
  
  olivas: i'd also agree not dataclasses
          dataclasses is supposed to be just frame objects
          these are utilities
  
  dschultz: I suppose, though the classes are frame objects
  
  olivas: right, so these act on frame objects which live in dataclasses, but that's true in some sense of nearly everything else
          maybe a separate project?
          we can make a new one 
  
  claudiok: how about `frame_object_diff` (if you want to keep it generic) or `GCD_diff`?
          as the project name
          and then I3GeometryDiff, I3CalibrationDiff
          as the frame object names
          the module and frame objects would all be in the new project
          you need this project to work on the frame objects anyway
          and no other project should ever need to access them directly
          so there won’t be any new dependencies 

