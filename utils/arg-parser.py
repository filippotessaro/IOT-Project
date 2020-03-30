import argparse
import os

# Construct the argument parser
ap = argparse.ArgumentParser()

# Add the arguments to the parser
ap.add_argument("-s", "--input_file", required=True,
   help="input log file")
ap.add_argument("-d", "--dest_dir", required=True,
   help="destination dir")

args = vars(ap.parse_args())

print("---START SCRIPT---")

input_file_str    = args['input_file']
output_dir = args['dest_dir']

in_file = open(input_file_str, "r")

print("Creating destination dir" , output_dir)
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

print("Creating log csv files")

f_neigh = open(output_dir + "/neighbour.csv", 'w+')
f_neigh.write("time,node_ID,Epoch,neigh_ID\n")

f_epoch = open(output_dir + "/epoch.csv", 'w')
f_epoch.write("time,node_ID,Epoch,num_nbr\n")

for line in in_file:
   if("New NBR" in line):
      '''2132122	ID:1	App: Epoch 0 New NBR 3'''
      line = line.replace("ID:", "")
      line = line.replace("App: Epoch", "")
      line = line.replace("New NBR", "\t")
      line = line.replace(" ", "")
      line = line.replace("\t", ",")
      f_neigh.write(line)
   elif ("finished Num NBR" in line):
      #parse end epoch
      '''2166837	ID:2	App: Epoch 0 finished Num NBR 0'''
      line = line.replace("ID:", "")
      line = line.replace("App: Epoch", "")
      line = line.replace("finished Num NBR", "\t")
      line = line.replace(" ", "")
      line = line.replace("\t", ",")
      f_epoch.write(line)


print("Closing files")
f_neigh.close()
f_epoch.close()

print("---END SCRIPT---")

