close all;clc;clear;

%% 参数设置
fs = 1000;
Ts = 1/fs;
t = 0: Ts :0.04-Ts;

x = sin(50*pi*t);
N = length(t);
y = fft(x)/N;   %快速傅里叶变换
z1 = abs(y);
z2 = angle(y);

if (mod(N,2)==0)
    k=-N/2:N/2-1;
else
    k = -(N-1)/2:(N-1)/2;
end;

f = k*fs/N; %fs/N是频率分辨率

for i = 1:N
    if(z1(i)<1E-10)
        z2(i)=0;
    end;
end;

figure();
subplot(3,1,1);
plot(t,x);
title('正弦函数一个周期的图像')
subplot(3,1,2);
stem(f,fftshift(z1));   %fftshift把fft变成左一半右一半
title('正弦函数频域幅值');
subplot(3,1,3);
stem(f,z2);