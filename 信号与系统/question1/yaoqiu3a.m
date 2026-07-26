clc;clear;close all;
fs1 = 100;
fs2 = 10;
ts1 = 1/fs1;
ts2 = 1/fs2;
t1 = 0:ts1:100;
t2 = 0:ts2:100;

x1 = exp(-t1);
x2 = exp(-t2);

N1 = length(t1);
if(mod(N1,2)==0)
    k1 = -N1/2:N1/2-1;
else
    k1 = -(N1-1)/2:(N1-1)/2;
end;
f1 = k1*fs1/N1;
N2 = length(t2);
if(mod(N2,2)==0)
    k2 = -N2/2:N2/2-1;
else
    k2 = -(N2-1)/2:(N2-1)/2;
end;
f2 = k2*fs2/N2;

%非周期信号要×周期
xf1 = fft(x1)*ts1;
xf2 = fft(x2)*ts2;

figure(1);
subplot(3,1,1);
plot(t1,x1);
title('exp(-t)信号图像');
subplot(3,1,2);
plot(f1,fftshift(abs(xf1)));
title('exp(-t)信号品频域幅值图像');
xlim([-5,5]);
subplot(3,1,3);
plot(f1,fftshift(angle(xf1)));
xlim([-5,5]);

figure(2);
subplot(3,1,1);
plot(t2,x2);
title('exp(-t)信号图像');
subplot(3,1,2);
plot(f2,fftshift(abs(xf2)));
title('exp(-t)信号品频域幅值图像');
xlim([-5,5]);
subplot(3,1,3);
plot(f2,fftshift(angle(xf2)));
xlim([-5,5]);

F = 1./(1+1i*2*pi*f1);
figure(3);
subplot(2,1,1);
plot(f1,fftshift(abs(xf1)),'r');
hold on;
plot(f2,fftshift(abs(xf2)),'b');
plot(f1,abs(F),'g');
hold off;
xlim([-50,50]);
title('两个频域幅频特性曲线');
subplot(2,1,2);
plot(f1,fftshift(angle(xf1)),'r');
hold on;
plot(f2,fftshift(angle(xf2)),'b');
plot(f1,angle(F),'g');
hold off;
title('两个频域相频特性曲线')
xlim([-50,50]);

t3 = 0:ts2:10;
x3 = exp(-t3);
N3 = length(t3);
if(mod(N3,2)==0)
    k3 = -N3/2:N3/2-1;
else
    k3 = -(N3-1)/2:(N3-1)/2;
end;
ft3 = fft(x3)*ts2;
f3 = k3*fs2/N3;

figure(4);
subplot(2,1,1);
plot(t3,x3);
title('定义域0-10的exp(-t)');
subplot(2,1,2);
plot(f3,fftshift(abs(ft3)));
