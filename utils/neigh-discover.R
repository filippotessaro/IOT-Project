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
# Read line file
processFile = function(filepath) {
  con = file(filepath, "r")
  while ( TRUE ) {
    line = readLines(con, n = 1)
    if ( length(line) == 0 ) {
      break
    }
    print(line)
  }
  
  close(con)
}


#-------------------------------------

df <- read.table("../dst_test/epoch.csv", 
                 header = TRUE,
                 sep = ",")

# list of nodes
nodes = c(2, 5, 10, 20, 30, 50)


# computes the offered load
neighbour.discovery.rate <- function(df, ID, lower.bound, upper.bound) {
  temp.df = subset(df, ( Epoch >= lower.bound & Epoch <= upper.bound))
  
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

by_cyl <- filtered.df %>% group_by(Epoch) %>% summarise(num_nbr = sum(num_nbr), var_num_nbr = var(num_nbr))


#-------------------------------------
#--------------------------------------

df <- data.frame()

#nodes = c(2, 5, 10, 20) # test
nodes = c(2, 5, 10, 20, 30, 50)

# computes the offered load
neighbour.discovery.rate <- function(df, ID, lower.bound, upper.bound) {
  temp.df = subset(df, ( Epoch >= lower.bound & Epoch <= upper.bound))
  
  return(temp.df)
}


for(i in nodes){
  print(paste("Preproc file:",i, sep=" "))
  file.name = paste(i,"epoch.csv",sep="")
  print(file.name)
  temp.df = read.table(paste("../dst_test/normal_tx_10task/", file.name, sep=""),
                       header = TRUE,
                       sep = ",")
  temp.df$config_nodes = rep(i, nrow(temp.df))
  
  # if first time 
  # bind cols names
  if (length(df) == 0){
    df = temp.df
  } else{
    df <- rbind(df, temp.df)
  }
  
}

# remove first 10 epoch and over 100
filtered.df = neighbour.discovery.rate(df,1,10,100)

# summarise by nodes and not in percentages
# by_epoch <- filtered.df %>% group_by(config_nodes, Epoch) %>% summarise(num_nbr = num <- sum(num_nbr))

by_epoch <- filtered.df %>% group_by(config_nodes, Epoch) %>% summarise(num_nbr = num <- (sum(num_nbr)/mean(config_nodes))*100)

by_epoch <- by_epoch %>% group_by(config_nodes) %>% summarise(avg = mean(num_nbr))

#group_by(filtered.df, config_nodes, Epoch) %>% 
  #summarise(GroupVariance=var(num_nbr), TotalCount=sum(num_nbr))

nodeslabel = c("2 Nodes", "5 Nodes", "10 Nodes", "20 Nodes", "30 Nodes", "50 Nodes")
 
#counts <- table(by_epoch$avg)
xx <- barplot(by_epoch$avg, main="Neighbour Dicovery Rate",names.arg=nodeslabel, horiz=FALSE, cex.names=0.7)
## Add text at top of bars
#abline(h=0)
text(x = xx, y = by_epoch$avg, label = as.integer(by_epoch$avg), pos=3, col = "red")


library(tidyverse)

by_count <- filtered.df %>% group_by(config_nodes, Epoch) %>% summarise(num_nbr = sum(num_nbr))


tibble(nodeslabel, by_epoch$avg) %>% 
  mutate(lab_loc = by_epoch$avg
         , col = factor(sign(by_epoch$avg))) %>% 
  ggplot(aes(x =  reorder(nodeslabel, -by_epoch$avg) , y = by_epoch$avg)) +
  geom_col(aes(fill = col)) +
  geom_label(aes(y = lab_loc, label = round(by_epoch$avg, 2))) +
  scale_fill_manual(name = 'sign', values = c('1' = '#6FAA46')) +
  ylab('Percentage') +
  xlab('Nodes') +
  ggtitle('Neighbour Dicovery Rate') +
  theme(plot.title = element_text(hjust = 0.5))
