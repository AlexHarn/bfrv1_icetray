/**
 *  copyright  (C) 2007
 *  the icecube collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 */

#include "gulliver/utilities/ordinal.h"
#include <stdio.h>

// convenience function for log messages
const char* ordinal(unsigned int i){
    static char myordinal[256];
    const char *suffix = "th";
    int last_digit = (i%10);
    int next_to_last_digit = (i%100)/10;
    if ( next_to_last_digit != 1 ){
        // so we'll have 1st, 2nd, 3rd, 11th, 12th, 13th, 21st, 22nd, 22rd, etc.
        switch( last_digit ){
            case 1: suffix="st"; break;
            case 2: suffix="nd"; break;
            case 3: suffix="rd"; break;
            default: break;
        }
    }
    sprintf(myordinal,"%d%s",i,suffix);
    return myordinal;
}
