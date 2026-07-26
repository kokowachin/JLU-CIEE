clc;close all;clear;
Fs = 6000;
ts = 1/Fs;
t = 0:ts:0.1;
x = 10*cos(200*pi*t)+8*cos(600*pi*t);

fp = 150;%通带fp
fs = 200;%阻带fs
wp = 2*pi*fp/Fs;
ws = 2*pi*fs/Fs;
Bt = abs(wp-ws);
N0 = ceil(6.2*pi/Bt);
N1 = N0 + mod(N0+1,2);
wc = (wp+ws)/2;
hn = fir1(N1-1,wc/pi,'low',hanning(N1));

figure();
freqz(hn,1);
y=conv(x,hn);

figure();
subplot(2,1,1);
plot(x);
subplot(2,1,2);
plot(y);

N=length(t);
xt=fft(x)/N;
Nm = N+N1-1;
yt=fft(y,Nm);
f0 = (-N/2:N/2-1)*Fs/N;
f1 = (-Nm/2:Nm/2-1)*Fs/Nm;

figure();
subplot(2,1,1);
plot(f0,fftshift(abs(xt)));
subplot(2,1,2);
plot(f1,fftshift(abs(yt)));