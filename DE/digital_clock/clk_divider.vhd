library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity clk_divider is
    port (
        clk_in    : in  std_logic; -- 2.048MHz 晶振输入
        clk_1hz   : out std_logic; -- 1Hz 输出
        clk_1khz  : out std_logic; -- 1kHz 输出
        clk_500hz : out std_logic  -- 500Hz 输出
    );
end clk_divider;

architecture rtl of clk_divider is
    -- 计数器信号定义 (2.048MHz = 2048000Hz)
    -- 1Hz:   周期 2048000，半周期翻转阈值为 1024000 (0 到 1023999)
    -- 1kHz:  周期 2048，半周期翻转阈值为 1024 (0 到 1023)
    -- 500Hz: 周期 4096，半周期翻转阈值为 2048 (0 到 2047)
    signal cnt_1hz   : integer range 0 to 1023999 := 0;
    signal cnt_1khz  : integer range 0 to 1023 := 0;
    signal cnt_500hz : integer range 0 to 2047 := 0;

    -- 寄存器输出信号
    signal out_1hz   : std_logic := '0';
    signal out_1khz  : std_logic := '0';
    signal out_500hz : std_logic := '0';
    
begin
    process(clk_in)
    begin
        if rising_edge(clk_in) then
            -- 1Hz 分频逻辑
            if cnt_1hz = 1023999 then
                cnt_1hz <= 0;
                out_1hz <= not out_1hz;
            else
                cnt_1hz <= cnt_1hz + 1;
            end if;

            -- 1kHz 分频逻辑
            if cnt_1khz = 1023 then
                cnt_1khz <= 0;
                out_1khz <= not out_1khz;
            else
                cnt_1khz <= cnt_1khz + 1;
            end if;

            -- 500Hz 分频逻辑
            if cnt_500hz = 2047 then
                cnt_500hz <= 0;
                out_500hz <= not out_500hz;
            else
                cnt_500hz <= cnt_500hz + 1;
            end if;
        end if;
    end process;

    -- 将内部寄存器信号连接到输出端口
    clk_1hz   <= out_1hz;
    clk_1khz  <= out_1khz;
    clk_500hz <= out_500hz;
    
end rtl;