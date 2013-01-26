args <- commandArgs(trailingOnly=TRUE)
print(paste("OMP P1 Threads Variable, Args: ", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('length', 'threads', 'ops', 'time')
data <- subset(rawdata, length == args[2], select=c(length, threads, time))
if (length(data) < 1) {
    q()
}

data <- data[with(data, order(threads)), ]

jpeg(args[3], width=1024, height=1024)
plot(data$threads, data$time, type="o", main=paste(args[1], ":",args[2], sep=""), xlab="Threads", ylab="Time")

dev.off()
