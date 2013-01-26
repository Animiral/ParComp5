args <- commandArgs(trailingOnly=TRUE)
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('n', 'p', 'time')

ps <- unique(rawdata$p)

jpeg(args[2], width=1024, height=1024)
plot(rawdata$n, rawdata$time, type="n", xlab="n", ylab="Time")

for (i in 1:length(ps)) {
	data <- subset(rawdata, p==ps[i])
	lines(data$n, data$time, col=i)
}

legend(1, max(rawdata$time), ps, col=1:length(ps), lty=1, pch=1)
dev.off()