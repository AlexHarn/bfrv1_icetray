#ifndef ORDINAL_H_INCLUDED
#define ORDINAL_H_INCLUDED

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

/**
 * utility function to pretty-print ordinal numbers
 * So 1 returns "1st", 5 returns "5th", 122 returns "122nd" etc..
 */
const char* ordinal(unsigned int i);

#endif /* ORDINAL_H_INCLUDED */
