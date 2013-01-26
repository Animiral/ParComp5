args <- commandArgs(trailingOnly=TRUE)
print(paste("MPI P2, Args: ", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('n', 'p', 'time')

ps <- unique(rawdata$p)

if ( length(rawdata$n) < 1) {
    q()
}

jpeg(args[2], width=1024, height=1024)
plot(rawdata$n, rawdata$time, type="n", xlab="n", ylab="Time")

for (i in 1:length(ps)) {
	data <- subset(rawdata, p==ps[i])
    data <- data[with(data, order(n)), ]
    if ( length(data$n) > 0) {
    	lines(data$n, data$time, col=i)
    }
}

legend(1, max(rawdata$time), ps, col=1:length(ps), lty=1, pch=1)
dev.off()
