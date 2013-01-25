args <- commandArgs(trailingOnly=TRUE)
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('length', 'threads', 'ops', 'time')
data <- subset(rawdata, threads == args[2], select=c(length, threads, time))
jpeg(args[3])
plot(data$length, data$time, type="o", main=paste(args[1], ",\nThreads:",args[2], sep=""), xlab="Length", ylab="Time")

dev.off()
