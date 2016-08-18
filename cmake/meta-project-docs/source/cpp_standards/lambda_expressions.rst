Lambda Expressions
-----------------

* "A lambda expression is just that: an expression. It is part of the source code" [Meyer].
* In other words, we can say that a lambda expression allows to write a function inside of another function.

``auto`` can be used as a ``lambda expression``, as ``type deduction`` and to handle STL containers.

``auto`` as a lambda expression
"""""""""""""""""""""""""""""""

If you place the blow expression inside of a ``main()`` function (for example), this will work::

   const auto lambda_momentum = [] (const double &px,
                                     const double &py){
    const double pz=px*0.6+py*0.8;
    const double &momentum=std::sqrt(p x*px+py*py+pz*pz);
    return momentum;
  };

    
``auto`` as type deduction
""""""""""""""""""""""""""

``auto`` can be used as a type. This is very useful when objects are very large and hard to handle. 
For example: ``std::map<std::string,int>::iterator``::

   for (auto it=StringIntMap.begin();
          it!=StringIntMap.end(); it++){
        std::cout<<"Key: "<<it->first<<" Value "<<it->second<<std::endl;
   }

In the above example, ``auto`` does not work as lambda expression, this works as type deduction.	   

   
Use a lambda expression to handle a STL container
"""""""""""""""""""""""""""""""""""""""""""""""""

You always should prefer to use STL algorithms instead of handwritten calculations [Sutter&Alexandrescu]. Below an example of how to use lambda expressions to manipulate a STL vector::

  std::vector<int>::iterator first_even = //this line can be replaced by "auto", used as Type Deduction                                   
      std::find_if(IntVect.begin(), IntVect.end(), [](int it) {
      return it % 2==0;
    });

  std::cout<<"The first even number is "<<*first_even<<std::endl;
 

References
""""""""""

[Meyer] Scott Meyers, http://ftp.factor.lg.ua/books/Effective_Modern_C__.pdf
[Sutter&Alexandrescu] Herb Sutter and Andrei Alexandrescu,  C++ Coding Standards. 

Full examples:
http://code.icecube.wisc.edu/projects/icecube/browser/IceCube/sandbox/CodeSprints/August2016/lambdas.cc
