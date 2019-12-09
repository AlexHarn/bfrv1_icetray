#!/usr/bin/env python

"""
Query every dataset (after 2008) in the simprod database to ensure that they
do not raise errors. This does _not_ check whether the result is actually
correct.

Note that this should not be an automated test, since it a) requires access to
the iceprod1 database, and b) puts quite a lot of load on the database.
"""

from icecube.weighting import weighting
import pprint

known_bad = {
	# ice model systematics with missing info in database
	20067, 20057, 20056, 20060,
}

with weighting.simprod_cursor() as cursor:
	cursor.execute("SELECT dataset_id from dataset WHERE dataset_id > 8790 ORDER BY dataset_id DESC")
	all_datasets = [i for i, in cursor.fetchall()]
	for dataset_id in all_datasets:
		try:
			weighting.from_simprod(dataset_id)
		except:
			steering = weighting.get_steering(cursor, dataset_id)
			# nwandkowsky's private simulation is outside the scope
			if 'nancy' in steering.get('TARGET', {}):
				continue
			# someone from IceTop needs to fix IceProd2-style IceTop configs
			if steering.get('category', None) == 'CORSIKA-icetop':
				continue
			# also, IceProd1 IceTop sets are weird and not really supported
			is_icetop = True
			for triggerword in ('jgonzalez', 'tamburro', 'icetop', 'topsimulator'):
				if triggerword in str(steering).lower():
					break
			else:
				is_icetop = False
			if is_icetop:
				continue
			# early iceprod2 datasets not in the iceprod1 db are also out of scope
			if 'MCPE_dataset' in steering:
				parent = int(steering['MCPE_dataset'])
			else:
				parent = steering.get('inputdataset')
			
			if parent in known_bad or parent not in all_datasets:
				continue
			# IceTop weighting not implemented for anything other than E^-1
			parent_steering = weighting.get_steering(cursor, parent)
			if parent_steering.get('CORSIKA::eslope', -1) != -1:
				continue
			
			print("Dataset {} is broken".format(dataset_id), parent)
			pprint.pprint(steering)
			pprint.pprint(weighting.get_steering(cursor, parent))
			# cursor.execute("SELECT cparameter_id, type, value FROM cparameter where dataset_id=%s", (dataset_id))
			# print(cursor.fetchall())
			raise
		print("Dataset {} is okay".format(dataset_id))
