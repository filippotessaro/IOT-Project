library(ggplot2)
library(dplyr)
suppressPackageStartupMessages(library("argparse"))


#------------------------------------------
# create parser object
parser <- ArgumentParser()

# specify our desired options 
# by default ArgumentParser will add an help option 

parser$add_argument("-i", "--in_dir",
                     help="Input csv Folder")
parser$add_argument("-o", "--out_dir", 
                    help="Output Plot and result Folder")
parser$add_argument("-n", "--node", type="integer", default=2, 
                    help="Number of processed nodes")

parser$add_argument("-l", "--lower", type="integer", default=10, 
                    help="Lower epoch Bound")

parser$add_argument("-u", "--upper", type="integer", default=100, 
                    help="Upper epoch Bound")

# get command line options, if help option encountered print help and exit,
# otherwise if options not found on command line then set defaults, 
args <- parser$parse_args()

# print some progress messages to stderr if "quietly" wasn't requested


in_dir = args$i
print(in_dir)

#------------------------------------------



df <- read.table("../dst_test/epoch.csv", 
                 header = TRUE,
                 sep = ",")

# list of nodes
nodes = c(1,2)


# computes the offered load
neighbour.discovery.rate <- function(df, ID, lower.bound, upper.bound) {
  temp.df = subset(df, (node_ID == ID & Epoch >= lower.bound & Epoch <= upper.bound))
  
  return(temp.df)
}

filtered.df = neighbour.discovery.rate(df,1,10,100)

pcr <- ggplot(filtered.df, aes(x=Epoch, y=num_nbr, colour=node_ID)) +
  geom_line() +
  geom_point() +
  xlab('# epoch') +
  ylab('# discovered neighbour') +
  labs(color="ID")
#ylim(c(0, 1))
#ggsave(paste(res.folder, '/normal-pcr_', n.nodes, '.pdf', sep=''), width=16/div, height=9/div)
print(pcr)

by_cyl <- filtered.df %>% group_by(node_ID) %>% summarise(num_nbr = mean(num_nbr), var_num_nbr = var(num_nbr))


