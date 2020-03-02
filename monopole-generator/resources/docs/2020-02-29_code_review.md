# Monopole-Generator Code Review

Sebastian Fiedlschuster, 2020-02-29

https://github.com/fiedl/monopole-generator/issues/1


## Version

This review refers to the monopole-generator project in the following version:
- https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6
- http://code.icecube.wisc.edu/svn/projects/monopole-generator/trunk@173007

Compiled against:
https://github.com/IceCube-SPNO/IceTrayCombo/tree/029c4839e44d8694f992bae92e839a8b288a6b90


## Summary

Monopole Generator is a fairly small project with a good-to-understand purpose and implementation. Its overall structure is clear and well-designed. However, the impression was that the project has been modified several times, but lacked the necessary maintenance work during that time. Documentation and tests have been out of date and did not match the current implementation and interface anymore.

After the initial review, I've fixed the tests and [revised the documentation](https://github.com/fiedl/monopole-generator/blob/861be01/README.md). I've created [CI scripts](fiedl/monopole-generator-install) to build the project against combo and to run the tests. I've prepared the merge of the monopole-generator project into [IceTrayCombo](https://github.com/IceCube-SPNO/IceTrayCombo) and listed recommendations for future code improvements.

As I'm not familiar with topic itself, yet, I've only reviewed the code, not mathematics or physics of the implementation.


## About Code Reviews

General information about code reviews in icecube:
http://software.icecube.wisc.edu/documentation/general/code_reviews.html


## Criteria Checklist

### First Glance

- [x] Does [documentation](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/docs) exist (doxygen, rst)?

  Result: Documentation does exist in the [`resources/docs`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/docs) directory.

  The [wiki contains similar information](https://wiki.icecube.wisc.edu/index.php/Relativistic_Monopole_Simulation), which is not in sync, however, with the [`resources/docs`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/docs) directory.

  Also, a `README.md` in the project root is not present.

- [x] Do [example scripts](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/examples) exist?

  Result: There is one example script in [`resources/examples`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/examples).
  However, this script demos only one of the two modules provided by this project.

  - [ ] It would be nice to have a demo script that generates and propagates monopoles and their secondaries and displays these events in [steamshovel](https://github.com/IceCube-SPNO/IceTrayCombo/tree/master/steamshovel).

- [x] Do unit tests exist?

  Result: There are python tests in [`resources/test`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/test) and cxx tests in [`private/test`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/private/test), which is in agreement with the review criteria (see section *Directory structure* below).

- [x] Does the code compile without warnings in both debug and release mode?

  Result: [Yes](https://github.com/fiedl/monopole-generator/issues/1#issuecomment-591608865)

- [x] Do the tests compile without warnings?

  Result: [Yes](https://github.com/fiedl/monopole-generator/issues/1#issuecomment-591638327)

- [x] Do all tests succeed?

  Results: No.

  - [x] The [python tests do succeed](https://github.com/fiedl/monopole-generator/issues/1#issuecomment-591647580).
    - [ ] But: Some of them show exceptions for which they are testing. It would be nicer to suppress the misleading output.
  - [x] The [cpp tests do not succeed](https://github.com/fiedl/monopole-generator/issues/1#issuecomment-591654198).
    - [x] Fixed in: https://github.com/fiedl/monopole-generator/issues/5
    - [ ] Some of the tested exceptions are shown. It would be nicer to suppress the misleading output.

  [Test coverage](https://github.com/fiedl/monopole-generator/issues/1#issuecomment-592038694):
  The overall line coverage is 83%. These parts of the code are not covered:

  - [ ] the "correct decay" mechanism, which simulates pi particles in addition to positron cascades
  - [ ] `I3MonopoleRelativisticUtils::UpdateMPInfoDict`
  - [ ] `I3MonopoleRelativisticUtils::MPSpeedProfile`
  - [ ] a couple of sanity-check fatals

### Documentation

- [x] Does it explain in reasonable detail what the project does?

  Result: Yes

- [x] Is it understandable for non-experts?

  Result: Yes

- [x] Look at the output of `icetray-inspect`. Are the docstrings useful for a person reasonably acquainted with the purpose of the project? Is the usage clear?

  Result: No

  - [x] Fixed in: https://github.com/fiedl/monopole-generator/issues/6

Further remarks on documentation:

- The documentation in `resources/docs`, in the wiki, and the actual code base were not in sync. Parts of the documentation were outdated.
  - [x] Fixed in: https://github.com/fiedl/monopole-generator/issues/6

- There exist several wiki pages describing older versions of the monopole generator.
  - [ ] We should add a comment to the top of those pages referring to the new monopole-generator documentation.

### Directory Structure

The code should be organized according to http://software.icecube.wisc.edu/documentation/general/code_reviews.html#directory-structure:

- [x] [`public/<project_name>`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/public/monopole-generator) - All public header files of the project should go into this directory. Header files for internal use do not belong in here. This directory is optional: if there are no public header files, it does not need to exist.

  Result: ok.

- [x] [`private/<project_name>`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/private/monopole-generator) - The project’s implementation goes into this directory. Conventionally, tableio converters belong into private/<project_name>/converter.

  Result: ok

- [x] `private/pybindings` - This is the directory for the python bindings.

  Result: This directory does not exist. (For an impression, check [clsim's `private/pybindings` directory](https://github.com/claudiok/clsim/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/private/pybindings).)

- [x] [`private/test`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/private/test) - Unit tests go in here.

  Result: ok.

- [x] [`python`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/python) - All pure python library code goes in here. If it exists, it must contain an __init__.py file that also loads the C++ pybindings library and the project library itself.

  Result: ok.

- [x] [`resources/scripts`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/scripts) - This is the directory for utility scripts.

  Result: ok.

- [x] [`resources/test`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/test) - Python test scripts go into this directory. Scripts to be run as tests must work without command line parameters.

  Result: ok. The scripts call python unit tests without command line parameters.

- [x] [`resources/examples`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/examples) - All other scripts should be put into this directory. However, it is not a dumping ground for people’s scripts. Only useful, commented, example scripts should be here.

  Result: ok. The directory contains one script: [`PlotGeneratingDistributions.py`](https://github.com/fiedl/monopole-generator/blob/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/examples/PlotGeneratingDistributions.py). This script is not commented. But one can find an example on how to call the monopole-generator module there.

- [x] [`resources/docs`](https://github.com/fiedl/monopole-generator/tree/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/docs) - All documentation should be placed here (as the name suggests). Plain text README files are not useful since they will not show up on automatically generated documentation pages. The preferred form of documentation is either rst or Doxygen. This is also not a good place for pdf files and other reports.

  Result: The directory contains rst and dox files.

  Remarks:

  - [x] The file [`index.rst`](https://github.com/fiedl/monopole-generator/blob/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/docs/index.rst) contains a redundant copy of [`monopole-generator.rst`](https://github.com/fiedl/monopole-generator/blob/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/docs/monopole-generator.rst), but no content from [`monopole-propagator.rst`](https://github.com/fiedl/monopole-generator/blob/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/resources/docs/monopole-propagator.rst).
  - [x] Also I would suggest to have the information copied or synced to a `README.md` in the repository root.
  - [x] I'd like to have instructions on how to build the software, and how to build and execute the tests in the README and documentation.
  - [x] Fixed in: https://github.com/fiedl/monopole-generator/issues/6

Further remarks:

- [x] The file [`CMakeLists.txt`](https://github.com/fiedl/monopole-generator/blob/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/CMakeLists.txt) does refer to both cpp and python tests, which would suggest that all tests are included when running the whole icecube test suite.
- [ ] The file [`.mailinglist`](https://github.com/fiedl/monopole-generator/blob/f68f19ff7a4d8fead45b4d6ae345723da69edbf6/.mailinglist) does contain email addresses. However, I do not see, which software will use the file to generate mailing lists. Also, I cannot find information how to trigger the generation of mailing lists after changing the file content.

### Code Structure

- [x] Every function should fulfill one specific purpose.

  Result: No. Several functions may be split up in shorter functions with clear names and purposes. But the code is understandable and for a project of this small size, it's ok.

- [x] Dead code, blocks of code that are commented out or disabled by `#if 0 … #endif` blocks should be removed.

  Result: No. There are several blocks of dead code.

- [x] Implementations go into `.cxx` files, not headers.

  Result: ok.

- [x] Header files that are not part of the module interface (e.g. classes/functions used internally by the module) go into the private directory.

  Result: ok.

### Coding Standards

- [x] Consistent naming of variables, classes.

  Result: No.

  - [ ] The naming of variables needs to be improved.
  - [ ] Also, a consistent case convention for variable and method names should be applied: `helloworld` vs. `helloWorld` vs. `hello_world`.

- [x] variables should have meaningful names, but normal IceCube abbreviations like DOM are okay.

  Result: No. (But as the code base is not very long, it's not too harmful. However, improving the variable names would serve readability and save newcomers time when reading the code.)

### Readability

- [x] Can you follow the logic of the code?

  Result: Yes.

- [x] Code duplication should be avoided

  Result: ok.

- [x] Are error and warning messages understandable?

  Result: Improvement recommended.

  Some error messages (like "out of range") do not help to fix the issue without reading the code. This can be improved by providing more constructive and informative outputs.

### Usability

- [x] Are parameters documented and understandable?

  Result: No.

  The documentation of the parameters has not been up to date. Also, the descriptions listed in `icetray-inspect` did not match the descriptions from the documentation, and tended to be not very helpful without reading the documentation and the code.

  Improved in: https://github.com/fiedl/monopole-generator/issues/6

- [x] Are parameters useful?

  Result: Yes.

## Detailed Annotation

I have annotated the code in its reviewed state and listed recommendations for future improvements:

https://github.com/fiedl/monopole-generator/pull/4

PDF export: [2020-02-29_code_review_appendix.pdf](https://github.com/fiedl/monopole-generator/files/4271080/2020-02-29_code_review_appendix.pdf)


## Fixes After the Review

The first fixes after this review are collected in this pull request:

https://github.com/fiedl/monopole-generator/pull/7


## Archiving This Code Review

This code review can be accessed on github: https://github.com/fiedl/monopole-generator/issues/1

- [x] A copy of the review is documented in `resources/docs` of the monopole-generator code. ([source](http://software.icecube.wisc.edu/documentation/general/code_reviews.html#method-of-code-review-process))
- [x] The review is linked to the project's `ReST` docs (?). ([source](http://software.icecube.wisc.edu/documentation/general/code_reviews.html#method-of-code-review-process))

In order to sync this markdown file `2020-02-29_code_review.md` to the equivalent rst file `2020-02-29_code_review.rst`, use [pandoc](http://pandoc.org):

```bash
$ pandoc --from markdown --to rst resources/docs/2020-02-29_code_review.md > resources/docs/2020-02-29_code_review.rst
```


## Next Steps

- [ ] Merge the monopole-generator project into https://github.com/IceCube-SPNO/IceTrayCombo
- [ ] Implement improvements from annotations in https://github.com/fiedl/monopole-generator/pull/4
- [ ] Review and merge [q-ball-support feature](https://github.com/fiedl/monopole-generator/pull/8) by Sarah Pieper
- [ ] Review and coordinate implementation of a [proposal by Frederik Lauber](https://icecube-spno.slack.com/archives/GUFGWJKGC/p1582642417006700) to refactor the monopole propagator in a way that extracts the production of cascades into a separate part of the code.
