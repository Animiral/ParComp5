# Fixed Size
# Args: File Name, Desired Length, Out File
args <- commandArgs(trailingOnly=TRUE)
print(paste("cilk_by_threads.R, Args:", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('length', 'threads', 'chunk', 'time')





full_data <- subset(rawdata, length == args[2], select=c(length, threads, chunk, time))
if (length(full_data) < 1) {
    q()
}

jpeg(args[3], width=1024, height=1024)

vals <- vector()
yreistn <- 0
#yreistn <- max(full_data$time)
if (args[2] > 3000) {
	vals <- c(1, 20, 100)
	yreistn <- 0.0023
} else {
	vals <- c(1, 500, 3000)
	yreistn <- 0.025
}

plot(full_data$threads, full_data$time, type="n", main=paste(args[1], ",\nSize:",args[2], sep=""), xlab="Threads", ylab="Time", ylim=c(0,yreistn))
for (i in 1:length(vals)) {
	data <- subset(full_data, chunk == vals[i])
    data <- data[with(data, order(threads)), ]
    if (length(data) > 0) {
	    lines(data$threads, data$time, col=i)
    }      
}
legend(1, yreistn, cex=1.2, c(vals), col=c(1:length(vals)), pch=1, lty=1)









dev.off()
