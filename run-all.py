import os

nodes_list = [2, 5, 10, 20, 30, 50]

os.system("make")

print("Start simulations")

for nodes in nodes_list:
    print("Simulation:", nodes)
    os.system('cooja_nogui nd-test-mrm-' + str(nodes) + 'n.csc')
    print("Simulation ended storing...")
    os.system("python3 ./utils/arg-parser.py -i test.log -d dst_test/normal_tx_10task/ -n " + str(nodes)
              + "  -c ./test_dc.log -r dst_test/normal_tx_10task/raw")

