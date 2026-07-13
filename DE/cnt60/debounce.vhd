LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;   -- 使用整数计数，更简洁

entity debounce is
    generic (
        DEBOUNCE_TIME_MS : integer := 10   -- 消抖时间（毫秒），可调
    );
    port (
        clk     : in  std_logic;   -- 2.048 MHz
        key_in  : in  std_logic;   -- 按键输入（低电平有效）
        key_out : out std_logic    -- 消抖后输出一个单周期脉冲（高有效）
    );
end debounce;

architecture Behavioral of debounce is
    -- 计算计数最大值：DEBOUNCE_TIME_MS * 2.048e6 / 1000
    -- 2.048 MHz 下，1 ms 对应 2048 个周期
    constant MAX_CNT : integer := DEBOUNCE_TIME_MS * 2048;   -- 例如 10*2048=20480

    signal cnt       : integer range 0 to MAX_CNT - 1 := 0;
    signal pulse_flag : std_logic := '0';   -- 标记脉冲是否已输出
begin
    process(clk)
    begin
        if rising_edge(clk) then
            if key_in = '0' then           -- 按键按下（低有效）
                if cnt = MAX_CNT - 1 then
                    -- 达到消抖阈值，保持计数，不再增加
                    cnt <= cnt;
                    pulse_flag <= '1';      -- 标记已输出脉冲
                else
                    cnt <= cnt + 1;
                    pulse_flag <= '0';
                end if;
            else                           -- 按键释放
                cnt <= 0;
                pulse_flag <= '0';
            end if;
        end if;
    end process;

    -- 输出一个时钟周期的脉冲：刚达到阈值且尚未输出过脉冲时置'1'
    key_out <= '1' when (cnt = MAX_CNT - 1 and pulse_flag = '0') else '0';
end Behavioral;