clc;clear;close all;
N1=20;
N2=500;
n1 = 0:N1-1;
x1 = cos(0.84*n1)+cos(0.96*n1);
w1 = (-N1/2:N1/2-1)*2*pi/N1;
y1 = fft(x1);   %不需要幅度矫正，因为ts=1
figure();
subplot(2,1,1);
stem(n1,x1);
title('取20个点的信号');
subplot(2,1,2);
plot(w1,fftshift(abs(y1)));
xlim([-3.14,3.14]);

n2 = 0:N2-1;
x2 = cos(0.84*n2)+cos(0.96*n2);
w2 = (-N2/2:N2/2-1)*2*pi/N2;
y2 = fft(x2);
figure();
subplot(2,1,1);
stem(n2,x2);
title('取500个点的信号')
subplot(2,1,2);
plot(w2,fftshift(abs(y2)));
xlim([-3.14,3.14]);

x3 = [x1,zeros(1,480)];
y3 = fft(x3);
figure();
subplot(2,1,1);
stem(n2,x3);
title('在20个点信号的基础上扩展');
subplot(2,1,2);
plot(w2,fftshift(abs(y3)));
xlim([-3.14,3.14]);