args <- commandArgs(trailingOnly=TRUE)
print(paste("OMP P1 Length Variable, Args: ", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('length', 'threads', 'ops1', 'ops2', 'time')
data <- subset(rawdata, threads == args[2], select=c(length, threads, time))
if (length(data) < 1) {
    q()
}

data <- data[with(data, order(length)), ]

jpeg(args[3], width=1024, height=1024)
plot(data$length, data$time, type="o", main=paste(args[1], ",\nThreads:",args[2], sep=""), xlab="Length", ylab="Time")

dev.off()
