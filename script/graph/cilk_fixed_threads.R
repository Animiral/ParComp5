# Fixed Threads
# Args: File Name, Desired Threads, Out File
args <- commandArgs(trailingOnly=TRUE)
print(paste("CILK Fixed Threads, Args:", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('length', 'threads', 'chunk', 'time')
full_data <- subset(rawdata, threads == args[2], select=c(length, threads, chunk, time))
if (length(full_data$length) < 1) {
    q()
}
jpeg(args[3], width=1024, height=1024)

plot(full_data$length, full_data$time, type="n", main=paste(args[1], ",\nThreads:",args[2], sep=""), xlab="Size", ylab="Time")
vals <- unique(full_data$chunk)

for (i in 1:length(vals)) {
	data <- subset(full_data, chunk == vals[i])
    data <- data[with(data, order(length)), ]
    if (length(data) > 0) {
	    lines(data$length, data$time, col=i)
    }      
	
}

legend(1, max(rawdata$time), cex=0.8, vals, col=c(1:length(vals)), pch=1, lty=1)
dev.off()
