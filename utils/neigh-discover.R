library(ggplot2)
library(dplyr)
library(tidyverse)

suppressPackageStartupMessages(library("argparse"))


#------------------------------------------
# create parser object
parser <- ArgumentParser()

# specify our desired options 
# by default ArgumentParser will add an help option 

parser$add_argument("-i", "--in_dir",
                     help="Input csv Folder", default="../dst_test/scatter_20task")
parser$add_argument("-o", "--out_dir", 
                    help="Output Plot and result Folder", default ="../dst_test/out")
parser$add_argument("-m", "--mode", default="scatter", 
                    help="Burst or Scatter")

parser$add_argument("-l", "--lower", type="integer", default=10, 
                    help="Lower epoch Bound")

parser$add_argument("-u", "--upper", type="integer", default=100, 
                    help="Upper epoch Bound")

# get command line options, if help option encountered print help and exit,
# otherwise if options not found on command line then set defaults, 
args <- parser$parse_args()

# print some progress messages to stderr if "quietly" wasn't requested

in.folder <- args$i
res.folder <- args$o
mode <- args$m

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

# *************************************************
# *
# * DATA PRE-RPOCESSING AND PLOTTING
# *
# *************************************************

df <- data.frame()

nodes = c(2, 5, 10, 20, 30, 50)

#nodes = c(2, 5)
sims <- c(0,1,2,3,4)


# computes the offered load
neighbour.discovery.rate <- function(df, ID, lower.bound, upper.bound) {
  temp.df = subset(df, ( Epoch >= lower.bound & Epoch <= upper.bound))
  
  return(temp.df)
}

for (j in sims){
  for(i in nodes){
    print(paste("Preproc sim:",j, sep=" "))
    
    print(paste("Preproc file:",i, sep=" "))
    file.name = paste(i,"epoch.csv",sep="")
    folder = paste(in.folder, "_sim", j, sep="")
    folder = paste(folder, "/", sep="")
    print(file.name)
    temp.df = read.table(paste(folder, file.name, sep=""),
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
}


# remove first 10 epoch and over 100
filtered.df <- neighbour.discovery.rate(df,1,10,100)

by_epoch <- filtered.df %>% 
  group_by(config_nodes, Epoch) %>% 
  summarise(avg = mean(num_nbr), variance = var(num_nbr))

by_epoch <- filtered.df %>% 
  group_by(config_nodes) %>% 
  summarise(avg = mean(num_nbr), variance = var(num_nbr))

by_epoch <- by_epoch %>% 
  mutate(percentage = avg/(config_nodes-1) * 100)

nodeslabel = c("2 Nodes", "5 Nodes", "10 Nodes", "20 Nodes", "30 Nodes", "50 Nodes")

by_count <- filtered.df %>% group_by(config_nodes, Epoch) %>% summarise(num_nbr = sum(num_nbr))

p <- tibble(nodeslabel, by_epoch$avg) %>% 
  mutate(lab_loc = by_epoch$avg
         , col = factor(sign(by_epoch$avg))) %>% 
  ggplot(aes(x =  reorder(nodeslabel, by_epoch$config_nodes) , y = by_epoch$percentage)) +
  geom_col(aes(fill = col)) +
  geom_label(aes(label = round(by_epoch$percentage, 2)), position = position_stack(vjust = 1.0)) +
  geom_text(aes(label = round(by_epoch$avg, 2)), color = "white", size = 4, position = position_stack(vjust = 0.5)) +
  scale_fill_manual(name = 'sign', values = c('1' = '#6FAA46')) +
  ylab('% of neighbours discovered in epoch') +
  xlab('Nodes') +
  ggtitle( paste('ND Rate', mode, sep=" ")) +
  theme(plot.title = element_text(hjust = 0.5), legend.position = "none")

ggsave(paste(res.folder, '/neigh_', mode , '.pdf', sep=''))

# *************************************************
# *
# * DC analysis part
# *
# *************************************************
col_headings <- c('Nodes','AVG_ON', 'AVG_TX', 'AVG_RX', 'AVG_INT')
dc.df <- data.frame()

#names(dc.df) <- col_headings
#data.frame(Nodes=NA, AVG_ON=NA, AVG_TX=NA, AVG_RX=NA, AVG_INT=NA)[numeric(0), ]

for (j in sims){
  for(i in nodes){
    print(paste("Preproc file:",i, sep=" "))
    #file.name = paste(i,"power.log",sep="")
    #print(file.name)
    
    file.name = paste(i,"power.log",sep="")
    #folder = paste(in.folder, j, sep="")
    folder = paste(in.folder, "_sim", j, sep="")
    
    folder = paste(folder, "/raw/", sep="")
    print(file.name)
    
    file.name = paste(folder, file.name, sep="")
    
    temp.df <- data.frame()
    
    con <- file(file.name, "r")
    lines.list <- readLines(con,n=4)
    list.vals <- list()
    
    for(line in lines.list){
      tmp.list <- unlist(strsplit(line, " "))
      #list.vals
      print(tmp.list[5])
      list.vals[length(list.vals) +1 ] <- tmp.list[5]
    }
    close(con)
    
    temp.df <- data.frame(matrix(unlist(list.vals), nrow=1, byrow=F))
    
    temp.df$config_nodes = rep(i, nrow(temp.df))
    
    # if first time 
    # bind cols names
    dc.df <- rbind(dc.df, temp.df)
    
  }
}  

dc.df <- dc.df %>% group_by(config_nodes) %>% summarise(AVG_ON = mean(as.numeric(as.character(X1))), 
                                               AVG_TX = mean(as.numeric(as.character(X2))), 
                                               AVG_RX = mean(as.numeric(as.character(X3))), 
                                               AVG_INT = mean(as.numeric(as.character(X4))))



tibble(nodeslabel, dc.df$AVG_ON) %>% 
  mutate(lab_loc = dc.df$AVG_ON
         , col = factor(sign(dc.df$AVG_ON))) %>% 
  ggplot(aes(x =  reorder(nodeslabel, dc.df$config_nodes) , y = dc.df$AVG_ON)) +
  geom_col(aes(fill = col)) +
  geom_label(aes(label = round(dc.df$AVG_ON, 2)), position = position_stack(vjust = 1.0)) +
  ylab('% Radio ON over time') +
  xlab('Nodes') +
  ggtitle( paste('ND Average Duty Cycle', mode, sep=" "))+
  ylim(0, 100) +
  theme(plot.title = element_text(hjust = 0.5), legend.position = "none")

ggsave(paste(res.folder, '/avgON_', mode , '.pdf', sep=''))# *************************************************


#ggplot(data = dc.df, aes(x = dc.df$config_nodes , y = dc.df$X1)) +
#geom_col(aes(fill = dc.df$config_nodes)) +
#geom_label(aes(y = lab_loc, label = round(dc.df$X1, 2))) +
#geom_label(aes(y = lab_loc, label = round(by_epoch$percentage, 2))) +
#geom_text(aes(y = lab_loc, label = round(by_epoch$avg, 2)), color = "white", size = 4, position = position_stack(vjust = 0.5)) +
#scale_fill_manual(name = 'sign', values = c('1' = '#6FAA46')) +
#ylab('AVG ON') +
#xlab('Nodes') +
#ggtitle('AVG On') +
#theme(plot.title = element_text(hjust = 0.5), legend.position = "none")
  
