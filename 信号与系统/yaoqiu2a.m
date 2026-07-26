clc;clear;close all;
fs1 = 400;
fs2 = 4000;
ts1 = 1/fs1;
ts2 = 1/fs2;

%只研究混叠，不能泄露
t1 = 0:ts1:0.6-ts1;
t2 = 0:ts2:0.6-ts2;
N1 = length(t1);
N2 = length(t2);

k1 = -N1/2:N1/2-1;
f1 = k1*fs1/N1;
k2 = -N2/2:N2/2-1;
f2 = k2*fs2/N2;

y1 = (square(2*pi*5*t1)+1)/2;
y2 = (square(2*pi*5*t2)+1)/2;

y1f = fft(y1)/N1;
y2f = fft(y2)/N2;

subplot(2,1,1);
plot(t1,y1,'r');
title('两个矩形波');
hold on;
plot(t2,y2,'b');
hold off;
xlabel('时间');
ylabel('频率分量');
ylim([-0.5,1.5]);

subplot(2,1,2);
stem(f1,fftshift(abs(y1f)),'r');
hold on;
stem(f2,fftshift(abs(y2f)),'b');
hold off;
title('矩形波的频域幅值');
xlim([-100,100]);
xlabel('频率f/Hz');
ylabel('频率分量');