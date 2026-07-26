close all;clc;clear;

%防止泄露：截断整周期/多测一会，测量越长，信号越好
%三角窗：主瓣宽且矮，旁瓣矮；
%矩形窗：主板高且细，旁瓣多且相对高
%% 参数设置
fs = 1000;
Ts = 1/fs;
t = 0: Ts :0.15-Ts;

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


M = 100000;
y3 = fft(x,M)/N;    %提高的不是分辨率，是密度，密度的提高使其包络线更光滑
z3 = abs(y3);       %加上方窗之后的为y3
f2 = (-M/2:M/2-1)*fs/M;


figure();
subplot(3,1,1);
plot(t,x);
title('正弦函数非整周期图像')
subplot(3,1,2);
stem(f,fftshift(z1));   %fftshift把fft变成左一半右一半
title('频域幅值');
subplot(3,1,3);
stem(f,z2);
title('相频特性图')

x2 = x.*bartlett(N)';%三角窗泄露状况这是在和三角窗作乘积
fx2 = abs(fft(x2))/N;
fx2m = abs(fft(x2,M))/N;
fm = f2;

figure();
subplot(1,2,1);
plot(f2,fftshift(z3),'r');%扩散到10000的作包络线
hold on;
stem(f,fftshift(z1),'b');
hold off;
title('加矩形窗后正弦函数幅频特性图');
xlim([-100,100]);

subplot(1,2,2);
plot(fm,fftshift(fx2m),'r');
hold on;
stem(f,fftshift(fx2),'b');
hold off;
title('加三角窗后正弦函数幅频特性图');
xlim([-100,100]);
