clc;close all;clear;
Fs = 5000;
ts = 1/Fs;
t = 0:ts:4-ts;
x1 = 10*cos(200*pi*t)+8*cos(600*pi*t);

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
Nm = N+N1-1;

y2 = fft(hn,Nm);
y3 = fft(x1,Nm);
y4 = ifft(y2.*y3);
t1 = -Nm/2:Nm/2-1;
w1 = y4(1:N);
y5 = fft(w1)/N;
f1 = (-N/2:N/2-1)*Fs/N;

figure();
subplot(2,1,1);
plot(t1,y4);
title('频域乘积后的信号');
xlim([-100,100]);
xlabel('n');
ylabel('h[n]');
subplot(2,1,2);
stem(f1,fftshift(abs(y5)));
xlabel('频率f/Hz');
ylabel('|X(f)|');

z2 = abs(y5);
y6 = conv(x1,hn);
w2 = y6(1:N);

figure();
subplot(2,1,1);
plot(t1,y6);
title('时域卷积后的信号')
xlim([-100,100]);
xlabel('n');
ylabel('h[n]');
subplot(2,1,2);
stem(f1,fftshift(abs(fft(w2)/N)));
xlabel('频率f/Hz');
ylabel('|X(f)|');