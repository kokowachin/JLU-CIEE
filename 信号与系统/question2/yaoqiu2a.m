clc;close all;clear;
Fs = 5000;
ts = 1/Fs;
t = 0:ts:4-ts;
x = 10*cos(200*pi*t);

fp = 150;%通带fp
fs = 200;%阻带fs
wp = 2*pi*fp/Fs;
ws = 2*pi*fs/Fs;
Bt = abs(wp-ws);
N0 = ceil(6.2*pi/Bt);
N1 = N0 + mod(N0+1,2);
wc = (wp+ws)/2;     %过渡带中心
hn = fir1(N1-1,wc/pi,'low',hanning(N1));

N =length(t);
xt = fft(x)/N;
k = -N/2:N/2-1;
f1=k*Fs/N;

figure();
subplot(2,1,1);
plot(t,x);
title('原信号图像');
xlabel('t');
ylabel('x(t)');
xlim([0,0.4]);

subplot(2,1,2);
stem(f1,fftshift(abs(xt)));
xlabel('频率f/rad');
ylabel('|X(f)|');

figure();
freqz(hn,1);
y = filter(hn,1,x);

figure();
subplot(2,1,1);
plot(t,y);
xlim([0,0.4]);
title('滤波后信号');
xlabel('t');
ylabel('y(t)');

subplot(2,1,2);
stem(f1,fftshift(abs(fft(y)/N1)));
xlabel('频率f/rad');
ylabel('|Y(f)|');