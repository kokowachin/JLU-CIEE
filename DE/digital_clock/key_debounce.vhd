library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity key_debounce is
    port (
        clk_1khz  : in  std_logic;                     -- 使用第一步产生的 1kHz 时钟
        key_in    : in  std_logic_vector(1 downto 0);  -- 两个物理按键输入 (默认高电平，按下低电平)
        key_pulse : out std_logic_vector(1 downto 0)   -- 输出消抖后的单周期高电平脉冲
    );
end key_debounce;

architecture rtl of key_debounce is
    -- 移位寄存器，用于持续采样按键引脚状态
    -- 16位在 1kHz 下等于 16ms 的时间窗口，非常适合消抖
    signal key0_shift : std_logic_vector(15 downto 0) := x"FFFF";
    signal key1_shift : std_logic_vector(15 downto 0) := x"FFFF";
    
    -- 记录按键上一次的稳定状态 ('1'表示松开，'0'表示按下)
    signal key0_state : std_logic := '1';
    signal key1_state : std_logic := '1';
    
begin
    process(clk_1khz)
    begin
        if rising_edge(clk_1khz) then
            -- 不断将最新的按键引脚状态移入寄存器最低位 (左移操作)
            key0_shift <= key0_shift(14 downto 0) & key_in(0);
            key1_shift <= key1_shift(14 downto 0) & key_in(1);

            ---------------------------------------------------
            -- 处理按键 0 (例如：MODE / 模式切换键)
            ---------------------------------------------------
            if key0_shift = x"0000" then      -- 连续 16ms 都是低电平，说明确实按下了
                if key0_state = '1' then      -- 如果上一个稳定状态是松开的 (边沿检测)
                    key_pulse(0) <= '1';      -- 吐出一个周期的脉冲
                    key0_state <= '0';        -- 更新稳定状态为已按下
                else
                    key_pulse(0) <= '0';      -- 手指一直按着不放，不再产生脉冲
                end if;
            elsif key0_shift = x"FFFF" then   -- 连续 16ms 都是高电平，说明确实松开了
                key0_state <= '1';            -- 更新状态为松开
                key_pulse(0) <= '0';
            else
                key_pulse(0) <= '0';          -- 处于中间抖动状态，不输出脉冲
            end if;

            ---------------------------------------------------
            -- 处理按键 1 (例如：ADD / 数值增加键)
            ---------------------------------------------------
            if key1_shift = x"0000" then
                if key1_state = '1' then
                    key_pulse(1) <= '1';
                    key1_state <= '0';
                else
                    key_pulse(1) <= '0';
                end if;
            elsif key1_shift = x"FFFF" then
                key1_state <= '1';
                key_pulse(1) <= '0';
            else
                key_pulse(1) <= '0';
            end if;
            
        end if;
    end process;
    
end rtl;