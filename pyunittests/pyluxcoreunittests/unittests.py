#! /usr/bin/python
# -*- coding: utf-8 -*-
################################################################################
# Copyright 1998-2013 by authors (see AUTHORS.txt)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

import sys
sys.path.append("../lib")
# To avoid .pyc files
sys.dont_write_bytecode = True

import os
import unittest
import pyluxcore

def LuxCoreLogHandler(msg):
	logFile=open("unittests.log","a")
	print(msg, file=logFile)
	logFile.close()

# To run the tests:
#
# python3 pyluxcoreunittests/unittests.py

def main():
	print("LuxCore Unit tests")

	# Clear the log file
	f = open("unittests.log","w")
	f.close()

	# Delete all images in the images directory
	folder = 'images'
	for f in [png for png in os.listdir(folder) if png.endswith(".png")]:
		filePath = os.path.join(folder, f)
		os.unlink(filePath)

	pyluxcore.Init(LuxCoreLogHandler)

	print("LuxCore %s" % pyluxcore.Version())
	
	suite = unittest.TestLoader().discover(".")
	unittest.TextTestRunner(verbosity = 2).run(suite)

if __name__ == "__main__":
    main();
