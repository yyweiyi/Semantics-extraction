#!/usr/bin/env python

import sys
import os
import os.path
import time
from subprocess import *

print "pwd : "
print os.getcwd()
pwd = os.getcwd()
root_path_directory = os.path.abspath(os.path.join(pwd, os.pardir));
svm_dir = root_path_directory + "/svm/"
print "svm path " + svm_dir
assert os.path.exists(svm_dir),"svm directory not found"

dir_path = os.path.dirname(os.path.realpath(__file__))
print "Dir_path : " + dir_path
if len(sys.argv) <= 1:
	print('Usage: {0} training_file [testing_file]'.format(sys.argv[0]))
	raise SystemExit

# svm, grid, and gnuplot executable files
is_win32 = (sys.platform == 'win32')
if not is_win32:
	svmscale_exe = dir_path+"/svm-scale"
	svmtrain_exe = dir_path+"/svm-train"
	svmpredict_exe = dir_path+"/svm-predict"
	grid_py = dir_path+"/grid.py"
	gnuplot_exe = "/usr/local/Cellar/gnuplot/5.0.5/bin/gnuplot"
else:
        # example for windows
	svmscale_exe = r"..\windows\svm-scale.exe"
	svmtrain_exe = r"..\windows\svm-train.exe"
	svmpredict_exe = r"..\windows\svm-predict.exe"
	gnuplot_exe = r"c:\tmp\gnuplot\binary\pgnuplot.exe"
	grid_py = r".\grid.py"

assert os.path.exists(svmscale_exe),"svm-scale executable not found"
assert os.path.exists(svmtrain_exe),"svm-train executable not found"
assert os.path.exists(svmpredict_exe),"svm-predict executable not found"
assert os.path.exists(gnuplot_exe),"gnuplot executable not found"
assert os.path.exists(grid_py),"grid.py not found"

train_pathname = sys.argv[1]
print "Train path name : " + train_pathname
parent_dir = os.path.abspath(os.path.join(train_pathname, os.pardir))
type_descriptor = os.path.basename(parent_dir)
assert os.path.exists(train_pathname),"training file not found"
file_name = os.path.split(train_pathname)[1]
scaled_file = parent_dir + "/" + file_name + ".scale"
model_file = svm_dir + "/svm_" + type_descriptor + ".model"
range_file = svm_dir + "/training_" + type_descriptor + ".range"

if len(sys.argv) > 2:
	test_pathname = sys.argv[2]
	file_name = os.path.split(test_pathname)[1]
	assert os.path.exists(test_pathname),"testing file not found"
	scaled_test_file = parent_dir + "/" + file_name + ".scale"
	predict_test_file = parent_dir + "/" + file_name + ".predict"

cmd = '{0} -s "{1}" "{2}" > "{3}"'.format(svmscale_exe, range_file, train_pathname, scaled_file)
print('Scaling training data...')
start_time_scaling = time.time()
Popen(cmd, shell = True, stdout = PIPE).communicate()
print("--- Scaling data took : %s seconds ---" % (time.time() - start_time_scaling))

cmd = '{0} -svmtrain "{1}" -gnuplot "{2}" "{3}"'.format(grid_py, svmtrain_exe, gnuplot_exe, scaled_file)
print('Cross validation...')
start_time_crossvalidation = time.time()
f = Popen(cmd, shell = True, stdout = PIPE).stdout

line = ''
while True:
	last_line = line
	line = f.readline()
	if not line: break
c,g,rate = map(float,last_line.split())

print('Best c={0}, g={1} CV rate={2}'.format(c,g,rate))
print("--- Cross validation took : %s seconds ---" % (time.time() - start_time_crossvalidation))

cmd = '{0} -c {1} -g {2} "{3}" "{4}"'.format(svmtrain_exe,c,g,scaled_file,model_file)
print('Training...')
start_time_training = time.time()
Popen(cmd, shell = True, stdout = PIPE).communicate()
print("--- training took : %s seconds ---" % (time.time() - start_time_training))

print('Output model: {0}'.format(model_file))
if len(sys.argv) > 2:
	cmd = '{0} -r "{1}" "{2}" > "{3}"'.format(svmscale_exe, range_file, test_pathname, scaled_test_file)
	print('Scaling testing data...')
	Popen(cmd, shell = True, stdout = PIPE).communicate()	

	cmd = '{0} "{1}" "{2}" "{3}"'.format(svmpredict_exe, scaled_test_file, model_file, predict_test_file)
	print('Testing...')
	Popen(cmd, shell = True).communicate()	

	print('Output prediction: {0}'.format(predict_test_file))