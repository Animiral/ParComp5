args <- commandArgs(trailingOnly=TRUE)
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('length', 'threads', 'ops', 'time')
data <- subset(rawdata, length == args[2], select=c(length, threads, time))
jpeg(args[3])
plot(data$threads, data$time, type="o", main=paste(args[1], ":",args[2], sep=""), xlab="Threads", ylab="Time")

dev.off()
