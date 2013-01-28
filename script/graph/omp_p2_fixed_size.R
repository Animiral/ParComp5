args <- commandArgs(trailingOnly=TRUE)
print(paste("omp_p2_fixed_size.R, Args ", args, sep=""))
rawdata <- read.csv(args[1], header = FALSE, sep=";")
names(rawdata) <- c('m', 'n', 'threads', 'time')
# data <- subset(rawdata, ((m==1) | (n==1) | (n==1024) | (m==2048) | (n==2048)))
# names(data) <- c('m', 'n', 'threads', 'time')
# data <- subset(data, (m*n) > 1024)

# BEGIN WURST

data <- subset(rawdata, n==1 & m==2048 )
names(data) <- c('m', 'n', 'threads', 'time')
data <- data[with(data, order(threads)), ]
jpeg(args[4], width=1024, height=1024)

#plot.new()
plot(rawdata$threads, rawdata$time, type="n", main=paste("By Threads\n", args[1], sep=""), xlab="Threads", ylab="Time", ylim=c(0,0.2))


ns <- unique(data$n)
ms <- unique(data$m)
counter <- 0
points <- vector()

for (i in 1:length(ms)) {
	filterM_data <- subset(data, data$m == ms[i])
	for(j in 1:length(ns)) {
		filterN_data <- subset(filterM_data, filterM_data$n == ns[j])
		counter <- counter+1
		lines(filterN_data$threads, filterN_data$time, col=(counter))
		points <- append(points, paste(ms[i], ", ", ns[j], sep=""))
	}
}

# END WURST










# BEGIN WURST

data <- subset(rawdata, n==2048 & m==1 )
names(data) <- c('m', 'n', 'threads', 'time')
data <- data[with(data, order(threads)), ]

ns <- unique(data$n)
ms <- unique(data$m)

for (i in 1:length(ms)) {
	filterM_data <- subset(data, data$m == ms[i])
	for(j in 1:length(ns)) {
		filterN_data <- subset(filterM_data, filterM_data$n == ns[j])
		counter <- counter+1
		lines(filterN_data$threads, filterN_data$time, col=(counter))
		points <- append(points, paste(ms[i], ", ", ns[j], sep=""))
	}
}

# END WURST










# BEGIN WURST

data <- subset(rawdata, n==1024 & m==1024 )
names(data) <- c('m', 'n', 'threads', 'time')
data <- data[with(data, order(threads)), ]

ns <- unique(data$n)
ms <- unique(data$m)

for (i in 1:length(ms)) {
	filterM_data <- subset(data, data$m == ms[i])
	for(j in 1:length(ns)) {
		filterN_data <- subset(filterM_data, filterM_data$n == ns[j])
		counter <- counter+1
		lines(filterN_data$threads, filterN_data$time, col=(counter))
		points <- append(points, paste(ms[i], ", ", ns[j], sep=""))
	}
}

# END WURST










# BEGIN WURST

data <- subset(rawdata, n==1024 & m==2048 )
names(data) <- c('m', 'n', 'threads', 'time')
data <- data[with(data, order(threads)), ]

ns <- unique(data$n)
ms <- unique(data$m)

for (i in 1:length(ms)) {
	filterM_data <- subset(data, data$m == ms[i])
	for(j in 1:length(ns)) {
		filterN_data <- subset(filterM_data, filterM_data$n == ns[j])
		counter <- counter+1
		lines(filterN_data$threads, filterN_data$time, col=(counter))
		points <- append(points, paste(ms[i], ", ", ns[j], sep=""))
	}
}

# END WURST











legend(1, .18, points, col=1:counter, pch=1, lty=1) 

#axis(1, at=1:length(data$threads), labels=c(data$threads))
dev.off()
