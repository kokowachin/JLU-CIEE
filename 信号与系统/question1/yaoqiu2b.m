% 研究泄露
clc;clear;close all;
fs1 = 1000;
ts1 = 1/fs1;

%只研究泄露
t1 = 0:ts1:0.6-ts1;
t2 = 0:ts1:0.5-ts1;
N1 = length(t1);
N2 = length(t2);

k1 = -(N1-1)/2:(N1-1)/2;
f1 = k1*fs1/N1;
k2 = -N2/2:N2/2-1;
f2 = k2*fs1/N2;

y1 = (square(2*pi*5*t1)+1)/2;
y2 = (square(2*pi*5*t2)+1)/2;

yf1 = fft(y1)/N1;
yf2 = fft(y2)/N2;

figure();
subplot(2,1,1);
plot(t1,y1);
title('未泄漏的原信号');
ylim([-0.5,1.5]);
subplot(2,1,2);
stem(f1,fftshift(abs(yf1)));
title('未泄漏的幅频特性图');
xlim([-100,100]);

figure();
subplot(2,1,1);
plot(t2,y2);
title('泄漏的原信号');
ylim([-0.5,1.5]);
subplot(2,1,2);
stem(f2,fftshift(abs(yf2)));
title('泄漏的幅频特性图');
xlim([-100,100]);