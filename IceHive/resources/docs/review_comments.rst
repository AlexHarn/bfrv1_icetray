.. highlight:: cpp

IceHive Code Review
===================

General
-------

* Project: HiveSplitter, IceHive branch
* Author: Marcel Zoll
* Sponser: David Boersma
* Consumers: everyone

* Version Reviewed: |revision|_

This code review was motivated by the request to run this as our main
splitting algorithm at the pole. Thus why *everyone* is listed as a consumer.
Because of the large change to all processing this could have, the review
will be detailed and nitpicky at times.

.. note::

    This review only covers the code itself, not the physics.

Documentation
-------------

Doxygen
'''''''

There is decent documentation of the two main modules (I3IceHive and 
I3HiveCleaning), so from a user perspective this is probably sufficient.

--- I hope so too :) DONE

rst
'''

There is no rst documentation at all, which needs to be remedied.
Some general user documentation is strongly recomended, and
detailed documentation of the algorithm would be welcome. I ask
for this because this is the standard documentation linked to from
http://software.icecube.wisc.edu/icerec_trunk/.

--- Well, this will be my first time writing .rst so please be a little bit patient with this;
I try to push the icetray and user interaction side of the doc into the rst and leave the implementation side to the .dox.
However, sometimes it is not so easy to destinguish between them. DONE

Examples
--------

There are simple examples for both splitting and cleaning.

A more complex example may prove useful, such as a real-life example
when running on a data or simulation file. What modules should go
before or after these modules?

--- The answer to this last question is this: before? Nothing! after? Whatever you want! ;
I'll try a little bit harder to illustrate the usecase in the .rst-doc but in general its hard to depict something general.

Tests
-----

Tests are still a work in progress and need more effort.

General Comments
----------------

It is easy to see that some of the code has been reviewed before
(in previous incarnations), as it is in decent shape regarding
our `coding standards`_.

Here are some general complaints:

OMKeyHash
'''''''''

Is there a reason we need another hash function for OMKeys?
Was the one in icetray not good enough? If you just needed OM and
String numbers and didn't want the PMT number, there is a ModuleKey
class in dataclasses for this.

For the lookup maps to work efficiently, I need a fast, compact and complete hash for OMkeys, which is also possible successive ordered and bijective onto a c++ simple type.
All this i can achieve with my function for OMkey-hashing 'SimpleIndex' as it only takes Str and OM-nbr and computes the hash by a linear function; also the hash is nativly computet to a simple int;
The OMKeyHash, as it is in icetray, is neither compact nor successive and stores additional information about the PMT which I am not using.
If I would choose to shitch I would either suffer:
* using maps instead of plain arrays, which is slow
* allocating possible too huge Lookup-tables, where I do never access certain positions/entries
Both things are not desireable.

--- I have implemented now some hints how to adapt these things and made the this a little bit more dynamic.

Fixed detector size
'''''''''''''''''''

Fixed sizes are a bad idea in general and especially now since upgrade
simulations have already begun. Unless you have an extremely good reason
for it, this needs to be set by the constructor at runtime. (#17 [sutter]_)

--- Hmmm, I know I promised to do this, however this is kind of a hard thing to do and might even slow the program when i implement the dynamism of this; ...
I just took another stroll through the program and found that currently IcecHive is just to hard coded and adapted to the IC86 detector as is. This involves:
* topology hardcodings in Hive-lib
* topology hardcodings in the generated Hive.dat files
* slower speed in the computation of OMKeyHashes with this dynamism
* I cannot quaranty compact but yet complete OMKeyHashes then (consider the new topology has 100 OMkeys per string than I waste 36 indexes in the wedging)
As all these things have to be taken care of if and only if the detector changes, and then have to be adapted to the specific topology more or less by hand (no, the computer has not yet learned to recognize shapes and geometries as logical/mathematical objects) I'd like to stick what I have here now.


Details in public headers
'''''''''''''''''''''''''

Templated code of course needs to be in header files, but over 90%
of the code for the project is in the public headers. Is there a way to hide
some of the implementation details in private files? If things are forced
to remain fully templated, possibly a break up into public and private
classes so the private class headers can actually be private.

--- I will move the portion of the code which I do not plane to share with others to the /private folder, which expecially includes the high-level I3Modules.

DANGER HARDCODING
'''''''''''''''''

Everytime I see ``DANGER HARDCODING`` I cringe a little more. Either a
value is a true constant which will never change, or it's an option
that may change. Remove the ambiguity. (#16 [sutter]_)

uint vs unsigned int
''''''''''''''''''''

There are many instances of each of these. You should pick one or the other
notation and stick with it. Also, please directly include ``sys/types.h``
instead of relying on icetray headers to include it for you.

--- Hmmm, I have now seen a lot of IceCube code- and have never once seen this being included.
So you want me to put this everywhere?

Some words on templates
'''''''''''''''''''''''

There are a lot of templates used here, probably more than is wise.
One thing I would recommend is that instead of ``Response`` being
a templated object, it could be an internal interface to access
any type of splittable object, with derived classes for each specific
object. This greatly reduces the burden of outside classes to support
exactly the syntax you want (meaning no modifying of dataclasses).
It should also allow you to make much of the code private, which would
provide a nicer interface for users.

(Copy from Email)
I will use the example from the call today, about time-like objects.  There is a way to do this either dynamically or statically:
::

  #include <iostream>
  #include "dataclasses/physics/I3RecoPulse.h"
  #include "dataclasses/physics/I3DOMLaunch.h"
  #include "simclasses/I3MCPulse.h"

  namespace dyn {
      /** Dynamic polymorphism */

      class TimeObject {
      public:
          virtual double GetTime() { }
      };
      class I3RecoPulseTimeObject : public TimeObject {
          I3RecoPulsePtr pulse_;
      public:
          I3RecoPulseTimeObject(I3RecoPulsePtr p) : pulse_(p) { }
          double GetTime() { return pulse_->GetTime(); }
      };
      class I3MCPulseTimeObject : public TimeObject {
          I3MCPulse pulse_;
      public:
          I3MCPulseTimeObject(I3MCPulse p) : pulse_(p) { }
          double GetTime() { return pulse_.time; }
      };
      class I3DOMLaunchTimeObject : public TimeObject {
          I3DOMLaunchPtr launch_;
      public:
          I3DOMLaunchTimeObject(I3DOMLaunchPtr l) : launch_(l) { }
          double GetTime() { return launch_->GetStartTime(); }
      };
  }

  namespace st {
      /** Static polymorphism via CRTP */

      template <typename T>
      class TimeObject {
          public:
              double GetTime() { static_cast<T*>(this)->GetTime(); }
      };
      class I3RecoPulseTimeObject : public TimeObject <I3RecoPulseTimeObject> {
          I3RecoPulsePtr pulse_;
      public:
          I3RecoPulseTimeObject(I3RecoPulsePtr p) : pulse_(p) { }
          double GetTime() { return pulse_->GetTime(); }
      };
      class I3MCPulseTimeObject : public TimeObject <I3MCPulseTimeObject> {
          I3MCPulse pulse_;
      public:
          I3MCPulseTimeObject(I3MCPulse p) : pulse_(p) { }
          double GetTime() { return pulse_.time; }
      };
      class I3DOMLaunchTimeObject : public TimeObject <I3DOMLaunchTimeObject> {
          I3DOMLaunchPtr launch_;
      public:
          I3DOMLaunchTimeObject(I3DOMLaunchPtr l) : launch_(l) { }
          double GetTime() { return launch_->GetStartTime(); }
      };
  }

  int main(int,char**){

      /** dymamically */

      // if input object is I3MCPulse
      I3MCPulse p(1.45);
      dyn::I3MCPulseTimeObject t(p);

      std::cout << t.GetTime() << std::endl;


      /** statically */

      // if input object is I3MCPulse
      st::I3MCPulseTimeObject t2(p);

      std::cout << t2.GetTime() << std::endl;

      return 0;
  }


Note that the choice of object type then comes down to an if statement.  I3Frame does have a type_name function for getting which type a key is, so a few ifs or a switch on the results of that would give you the appropriate object.

Depending on what you are trying to do, either dynamic or static polymorphism may be appropriate.  I would lean slightly towards dynamic, since you could write functions like:
::

  double GiveMeTheTime(dyn::TimeObject& t) { return t.GetTime(); }

where you can take any TimeObject and it will just work.  You can do the same statically, but it's not as streamlined:
::

  template <typename T>
  double GiveMeTheTime(st::TimeObject<T>& t) { return t.GetTime(); }

though maybe you want the compile time guarantees more than the easily readable code. 


--- I have to think about this and if such implementation is really useful,
I am kind of opposed to the dynamic thing, as this suggests splitting anything that is at a certain key in a frame, which fulfillst a given sugnature;
and figuring this out for every frame, a thus a lot of casts. and the user should actually know what he is about to split!
the static approach sounds more intresting, however this would not lift any templating functions, but just move it a little bit deeper into the code and associate it to the processed objects;
I need more time for this if I get arround to implement this.

Specific Comments
-----------------

HiveSplitter.h
''''''''''''''

* lines 17,21,23,24,29: Several headers seem to be included but not used.
  Cleanup is in order.
  -- removed DONE

* lines 41-61: Since these variables are public, do they need a trailing
  underscore?
  -- well, they all once recided in HiveSplitter and then were broken out into 
  this parameter-struct, so that they can be collectivly propagated to the splitter.
  The underscore is a residual, which kind of signaled me that this is still a real
  option. Rethinking it, the access by ``params_.parameterX`` the trailing underscore on 
  ``params_`` should suffice. <These are a lot instances in the code to eliminate> DONE

* lines 82-123: The data structures and helper functions don't seem to
  depend on anything within the HiveSplitter class, so why are they inside
  the class? Putting this in a private namespace would be better.
  -- Maybe a good idea to tidy up the code; this is mainly a residual
  from the implementation of TopologicalSplitter.
  It is broken out now; DONE
  
* line 138: Saying that the variable is static in a comment isn't a good
  idea. Either use the keyword static in the code, or it's not static.
  Perhaps you should call it fixed size? Though I'm not sure the comment
  is even necessary.
  -- this was a personal remark telling me what is done at configuration time and 
  what is done at runtime; replaced with a nicer comment; DONE

* lines 140,142: These two lookup tables are presented as c-style arrays
  using string and om indices. Would a hash_map be more appropriate for
  a lookup-based structure?
  --- Premisses: 
  1.These objects are created once at configuration time;
  2.Each one of them needs to store a double per entry
  3.The maps need to index the the full range of possible OMKeys squared (all possible combinations)
  4.the maps are in general irreducable, but can be reduced with certain restrictions (this follows by the multiple functionality they carry)
  
  Thereby the easiest way to solve this is by a plain two-dimensional array
  
  Advantages:
  1. Plain structure: direct access by index, not trough iterators
  2. Compactness: nothing is more compact in memory (without compression) than arrays (its just a series of memory adresses)
  3. Direct access: I can access the memory directly, just through the arrays-offset operators a[x][y]; the only faster way would be by knowing the memory adress directly
  
  So ion my eyes this is a very fast and very convenient way to structure these tables; the only inconvenience is that I have to go through my 'SimpleIndex'-Hashing functions, which is a fast thing (linear function).
  
  Every other implementations by std::map or vector<vector> forces me to go through iterators and will never be as compact as this is;
  
  All in all i'd rather stick with this implementation as it works and I see only disadvantages with my other choices.
  
  --- these are now fully dynamic objects DONE
  
* lines 289-290,685-686: Don't write namespace usings in a header file.
  (#59 [sutter]_)
  --- Yes I was naughty,
  when i restructured the code I was looking for a quick fix, and that is what I ended up with, never thought about the consequences;
  I gonna go write a lot of '::' now :)
  
  as concerning lines 685-686: This is function internal and should not radiate out to anything including this header;
  --- all namespaces are now inside functions or in cxx files DONE
  
* line 303: Prefer prefix increment. (#28 [sutter]_)
  --- ohhh well, I never got that; as I heard compilers would be smart enough to realize when it would make no diffenerence;
  I gonna wash the code then; DONE


* line 332: Do you want to create a new Hit or just a reference to the
  first hit in the list? Because this creates a new Hit object.
  --- first: this code was not written by me;
  However, what is created there is in fact a copy of the first Hit in the hit-series in that cluster;
  I agree that a const ref is much better in this case! :) DONE

* line 336: Use the key to erase directly instead of finding the iterator
  first.
  --- still not my code; but I agree :) DONE

* line 340: How much does the hinting matter for speed? And would it be
  better to do something like this::
  
    if (complete.empty())
      complete.insert(h)
    else
      complete.insert(--complete.end(),h)

  --- well as the cluster.complete is a (time-ordered)HitSet and you already know that the hit to be inserted
  can only be later than all other previous hits, the hinting is quite effective; the speed gained..?
  there should be some information about this somewhere in the web, as this seems quite a general problem;
  still the size of the set is not unreasonably huge so the effective gain should be not that much. DONE
      
* line 361: What happens when c.complete is empty? I'm pretty sure that's
  undefined behavior, and should be avoided.
  --- I do not see the undefined behaviour; inserting nothing into something leaves you with something;
  I am pretty sure the std::set::insert (http://www.cplusplus.com/reference/set/set/insert/) does take care of this case; the alternative would be unncessary safeguards
  DONE
  
* line 384: Can this be simplified to ``return(it1==end1)``? The way it
  is currently written, both it1 and it2 can have not reached then ends
  and it returns true, which is not desired (not that this case should
  occur, but just in case).
  --- hmmm, This made me review all the lines, which pointed me to bigger problem/loophole:
  objetcs which are compared are cluster.hits, which are of type std::list<Hit>.
  In the IsSubset-function algorithm by forward-iteration and comparision (operators == and <) is used;
  exspecially the use of "<" breaks down if the input is not already ordered;
  This breaks the generality of the function, however by construction in IceHive/HiveSplitter/TopologicalSplitter
  the clusters will always have only hits pusched into the .hits in time order. So the loophole is never triggered.
  TODO NOTE I will leave this for now and look later, if the cluster::hits can be made into a TimeOrderedHitSet in order to avoid this
  
* lines 395-397: First you call ``GetDistance(h1.domIndex, h2.domIndex)``,
  then just below that you call
  ``GetDistance(SimpleIndex2OMKey(h1.domIndex), SimpleIndex2OMKey(h2.domIndex))``.
  Is there a reason for the different calls?
  No there isn't. An artifect from a time, when i did not provide both interfaces yet; DONE

* line 405: It seems you could return a lot sooner if you tested that dist was
  NAN here, saving some computation.
  --- Congratulations, you found a bug;
  actually line 412 should have read::
    
    const bool eval = (in_dist && (particle_causal || photon_causal)) || displacement_causal;
    
  so unfortunatelly I cannot return early and have to do all the computations.
  Then again furtunatelly the bug was never triggered by the current choice of parameters.
  
  --- Because of this line change a bad bad BAD bug was found in the previous lines, reading::
    
    const bool in_vicinty= VicintyMap_[h1.pulseIndex][h2.pulseIndex];
  
  instead of ::
  
    const bool in_vicinty= VicintyMap_[h1.domIndex][h2.domIndex];

  DONE

* line 437: Fix the FIXME.
  --- I have given up on fixing this FIXME. it seemed like a great idear at the time, but has proven much more complicated
  than ever anticipated and would require EXTENSIVE rebuilding the code: DONE

* line 447: If you need pairs of numbers, why don't you just ask for
  input as a ``std::vector< std::pair< double, double > >``?

  --- this would require the user to pass things like this in python::
    from icecube.dataclasses import make_pair
  
    tray.AddModule("IceHive",
      Parameter = [make_pair(100, 100), make_pair(10, 10), make_pair(1,1)])

  I am not really shure about the advantage of the one over the other;
  NOTE if you strongly request it, I will change the interface!
  
* lines 515-517: Why are the hits extracted in one ordering, only to be
  reordered immediately?

  --- Good question: At the time I wrote the HitSorting Lib I just wanted to provide a single interface to the 'ExtractHits' which was retrieval-ordered.
  Later I realized it might be disireable to immediatelly time-order, but never implemented it to the lib.
  With the tools of templating now in my hand I gonna do just that now; ... Done

* line 548: A comment would be helpful to explain what is happening here.

  --- Well I would have to guess; this part of the machinery, which I inherited is not very clear, as it entangles many components at once
  and tries to reduce the number of calls and temp varianles to a minimum; (I can not imagine, whoever might want to do something like this in his/her code...)
  However, I promoted the CausalCluster::advanceTime to a member function HiveSplitter::AdvanceClusterInTime() and thereby get around the parsing of the instance.
  Still not very nice, but a OK compromise.
  Inserted comment at place::
  
    //each cluster is advanced in time:
    //removing all too old/expired hits, which cannot make any connections anymore;
    //concluded clusters, which do not have any connecting hits left, become Inactive and are put to the garbage
    //if the cluster is still active, try to add the Hit to the cluster
    
  DONE

* lines 549,573,622,642,661: ``erase()`` returns an iterator to the next
  element in the list. Please use it instead of getting the next iterator
  before erasing.
  --- So my task is to get rid of all these ugly ugly for-loops with the peaking 'next' operations and changing them to proper while loops? Yeay.
  While I have to say that I like the idea, I am afraid how much one can break with this;
  <=== making stoppoint at revision 122101
  ...DONE but I am afraid; 
  --- The fear was justified; I forgot a ++cluster at some place and created an infinity loop; corrected now; DONE 

* lines 690-693: Do not use $I3_SRC. Prefer $I3_BUILD.
  --- DONE

* lines 882-883: The comparison ``domTopo_A >= domTopo_B`` is done twice.
  It is better to use a proper if/else construct and do the comparison
  once.
  --- Here I have to disagree;
  By using the code as is, I can keep the code to two lines where the differences are quite
  visible and have my variables initialized with const at the same time;
  the price to pay is comparing two int's twice; DONE

HiveSplitter.cxx
''''''''''''''''

.. _vector-constructor:

* lines 28-40: The vectors can be filled in the constructor, so you can
  do this::
  
    static const double ic[] = {300., 300., 272.7, 272.7, 165.8, 165.8};
    static const double dc[] = {150., 150., 131.5, 131.5, 40.8, 40.8};
    static const double pingu[] = {150., 150., 144.1, 144.1, 124.7, 124.7, 82.8, 82.8};
    static const double v_ic[] = {100.,100.,100.,100.};
    static const double v_dc[] = {100.,100.,100.,100.,100.,100.};
    static const double v_p[] = {100,100.,100.,100.,100.,100.,100.,100.};
    HiveSplitter_ParameterSet::HiveSplitter_ParameterSet():
      multiplicity_(4),
      timeWindow_(2000.*I3Units::ns),
      timeStatic_(200.*I3Units::ns),
      timeCVMinus_(200.*I3Units::ns),
      timeCVPlus_(200.*I3Units::ns),
      timeCNMinus_(200.*I3Units::ns),
      timeCNPlus_(200.*I3Units::ns),
      selfconnect_(true),
      domSpacingsOpt_(false),
      SingleDenseRingLimits_(ic, ic + sizeof(ic) / sizeof(ic[0]) ),
      DoubleDenseRingLimits_(dc, dc + sizeof(dc) / sizeof(dc[0]) ),
      TripleDenseRingLimits_(pingu, pingu + sizeof(pingu) / sizeof(pingu[0]) ),
      SingleDenseRingVicinity_(v_ic, v_ic + sizeof(v_ic) / sizeof(v_ic[0]) ),
      DoubleDenseRingVicinity_(v_dc, v_dc + sizeof(v_dc) / sizeof(v_dc[0]) ),
      TripleDenseRingVicinity_(v_p, v_p + sizeof(v_p) / sizeof(v_p[0]) )
    { }

  Note that I used sizeof instead of a fixed length, which is preferred
  (magic numbers are disliked, and the compiler is smart).
  --- DONE

I3IceHive.h
'''''''''''

* lines 90,92: These pointers never get cleaned up, resulting in a memory
  leak. Perhaps you need a destructor? You should probably also initialize
  them to NULL. Or you can use a shared_ptr.
  --- True, until they where cleaned by the OS garbage-memory collection at program termination, which was also their desired lifetime.
  They will be properly destroyed now. DONE

* line 156: Are these internal or external helpers? If internal, they should
  be private.
  --- They are external. Moved them out to a IceHiveHelpers.h. DONE
  
* line 289: How do you know that the user chose not to apply TriggerSplitting?
  --- I don't. Corrected the parameter-correctness check and promoted warnings to log_fatal DONE

* line 361: Why use ``trigHierName_`` instead of ``triggerSplitterOpt_``?
  The way it is written, ``triggerSplitter_`` may not be a valid pointer.
  --- an artifect from times of old; corrected it; DONE
  
* lines 380,401,408: Remove lines that are commented out and no longer valid.
  --- DONE


Hive-lib.h
''''''''''

* line 15: Include the stdlib as a c++ header::

    #include <cstdlib>

  --- unsigned int are now in place instead of uint. DONE
  
* line 22: Check to make sure this is not already declared somewhere else
  (like ``sys/types.h``). Also, ``uint`` is a really common name to be
  adding to the global namespace.
  --- as requested in the general comments i converted uint => unsigned int. DONE
  
Hive-lib.cxx
''''''''''''

* There are a number of ``//std::cout`` lines, which should either
  be converted to log_trace or removed.
  --- hmmm, they are there for non frequent debugging reasons; (I vaguatly remember that I had logging-problems at that time also).
  DONE

* line 12: Include the stdlib as a c++ header.
  --- DONE
  
* lines 31-32: Consider using ``honey_[0]`` where appropriate instead of 
  an iterator.
  --- I'll obey. DONE
  
* lines 52-56: I again question the use of an iterator, instead of::

    if (honey_.size() <= ringnbr)
        return std::set<uint>();
    else
        return honey_[ringnbr];
        
  --- Seems I was very fond of iterators at that time. DONE

* lines 93-94,97,102: Better to get the ``honey_.size()`` directly,
  so we do one less function call and less addition operations.
  --- I'd rather not; the code has certain structure; where functions at the bottom build on functions which have been implemented at the top;
  Also the call internal structure gets more and more complicated; Giving certain operations some common names, actually helped me a lot to write this code, so I am hesitent to take them away again for one less call;
  As a compromise I put this function of GetNRings() to the header and asked the compiler to 'inline' it. DONE

* line 189: It is not guaranteed that ``combs->begin()`` returns a valid
  iterator. Check that it does not equal ``combs->end()`` before using it.
  --- Well, this function is depricated and not used anywhere in the rest of the code: kill Kill KILL (it's getting late); DONE

* line 192: You say in the documentation "-1 if not possible" but return 0.
  --- RIP; DONE

* lines 199-214: Instead of adding all strings then removing those already
  in the current comb, consider first getting the set of strings in the
  comb and only adding strings that do not match that. This prevents
  erasing after the fact, which is not very clean.
  --- RIP; DONE
  
* line 228: You say in the documentation "-1 if not possible" but return 0.
  --- DONE

* line 237: In all other loops ``scale`` starts at 1 instead of 0.
  --- actually not true; it starts from all kinds of indeces :)
  So if I remember this correctly (this function was one of my masterpieces and
  took me multiple hours to come up with),
  the idea is that on a hive all strings add their strings on rings to all other strings
  and redundant entries are successively removed. the ``scale`` is a varibales that tracks how many
  of such rings are processed. Last time I checked the algorithm worked :) DONE
  

* lines 220-282: Unless you have a very good speed reason, it is better to
  call ``ExpandToNextRing()`` ``scale_factor`` number of times. It is
  significantly simpler to do this, thus easier to test for correctness.
  --- I have removed ``ExpandToNextRing()`` as I concidered it also before an unnice function.
  Also there is no concern for speed here, because this is construction-functionality,
  which should be used only by users, which know what they are doing. DONE
  

* line 294: Did you mean to compare ``combs1`` and ``combs2``? Right now
  this will always return false.
  --- I was; eliminating another bug; DONE

* line 299: Prefer the copy constructor over assignment.
  --- learned something for teh future; DONE

* lines 445,456,467,472,483,488: Prefer ``&&`` instead of nested
  conditionals. (#20 [sutter]_)
  --- ...and it was so nicely and clearly structured :/ DONE

HitSorting.h
''''''''''''

* line 97: Allowing the InputIterators to be different classes
  means we might be comparing two completely different classes.
  This is fairly dangerous and probably not needed.
  --- see comment question below

* line 98: Why do you take iterators here, yet in ``SetsIdentical``
  you take ``ordered_set`` objects?
  --- Not my code: the idea was, I think, to be able to compare only subsets of sets too;
  I'll edit the funktion-signature, as it only occures once in the code and the sets complete sets are compared this is possible.
  DONE

* line 115: ``std::equal`` can replace ``SetsIdentical()``.
  (#84 [sutter]_)
  --- seems I rewrote the internals of ``std::equal`` <http://www.cplusplus.com/reference/algorithm/equal/> DONE

* line 249: Where do you create the vector that you're pushing back into?
  --- I don't, this is conveniently done for me. Whenever I access a new key in a
  ``std::map`` the std-constructor on the ``value-type`` is called.
  In this case the value-type is a vector which defaults to an construction of an empty vector. DONE

* lines 244-258: Should the if statement go inside the loop for less
  duplicated code?
  --- hmm, okay; also I installed some saveguards, which however might slow down the computations DONE
  
* line 341: Incorrect function name in debug statement.
  --- DONE

* line 362: Would it be better to iterate instead of using an index?
  --- german: ''das gibt sich nix'' -- there is no difference [in permormance, beauty, whatever]. I would leave it as is DONE

* lines 370-373: Suggest using ``std::accumulate()``.
  --- Nope; the iteration goes over a vector of deep objects, where the objects property should be summed; aka vector<Hits> and sum ofer Hits.charge()
  this is not possible with std::acummulate as it can only handle plain types. DONE

* line 390: Incorrect function name in debug statement.
  --- Here and all further: I restructured the code, so that the class HitSorting HitSortingFacility is now in fact usable; DONE

* line 391: What is ``GetRetrievalOrdered()``? Did you mean 
  ``GetRetrievalOrderedHits()``?
  --- see above

* line 392: What is ``sort()``? Is this ``std::sort()``?
  --- see above

* line 401: Fix the FIXME.
  --- see above

* line 409: Why use the iterator when you can use array notation?
  --- see above

* lines 405-421: Should the if statement go inside the loop for less
  duplicated code?
  --- see above
  
* line 428: Fix the FIXME.
  --- see above

HitSorting.cxx
''''''''''''''

* lines 50-53,58-61: Couldn't you just use the set copy constructor?
  --- I could! DONE

HiveCleaning.h
''''''''''''''

* There appears to be a lot of duplicate code (and comments) from 
  *HiveSplitter.h*. Perhaps this should be factored out? I'm not going
  to bother reviewing the duplicate code, so assume it has the same
  problems as in the other file.
  --- Your notion is right, big parts are copies, however they have to provide less functionality.
  I try to obey the pointers given in the previous text. DONE

* lines 138-139,205,277-278: Don't write namespace usings in a header file.
  (#59 [sutter]_)
  --- Removed when appearing outside functions. DONE

I3HiveCleaning.h
''''''''''''''''

* line 37: Does this need to inherit from I3Splitter? It doesn't look like
  it splits anything.
  --- An artifect; removed; DONE
  
* line 77: Cannot find implementation for ``ConfigureSplitters()``.
  --- removed DONE

TriggerSplitter.h
'''''''''''''''''

* line 306: Don't write namespace usings in a header file. (#59 [sutter]_)
  --- Withing a function so it should be okay DONE

TriggerSplitter.cxx
'''''''''''''''''''

* line 43: The vector can be filled in the constructor (as seen
  :ref:`above <vector-constructor>`).
  --- DONE
  
* lines 89-90: Prefer ``&&`` instead of nested conditionals. (#20 [sutter]_)
  --- DONE

pybindings/module.cxx
'''''''''''''''''''''

* line 25: Don't write namespace usings before an #include. (#59 [sutter]_)
  --- DONE


References
----------

.. [sutter] *C++ Coding Standards* (http://www.gotw.ca/publications/c++cs.htm), by Herb Sutter and Andrei Alexandrescu


.. _coding standards: http://software.icecube.wisc.edu/offline_trunk/code_standards.html


.. |revision| replace:: projects/HiveSplitter/branches/IceHive r121895
.. _revision: http://code.icecube.wisc.edu/projects/icecube/browser/IceCube/projects/HiveSplitter/branches/IceHive?rev=r121895


