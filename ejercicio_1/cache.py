#!/usr/bin/python3.5

import math

HL = "+" * 80

# parametros del cache
l1_size = 16*1024
l2_size = 128*1024
block_size = 32
address_bits = 8

M = 1
E = 2
S = 3
I = 0

READ_REQ = 0
WRITE_REQ = 1

class SetLRU:
    def __init__(self):
        self.lines = [Line(0,I), Line(0,I)]
    
                
    def touch_block(self, n):
        if n==1:
            self.lines[1].lru_bit = 1
            self.lines[0].lru_bit = 0
        elif n==0:
            self.lines[1].lru_bit = 0
            self.lines[0].lru_bit = 1
            
    def touch_tag(self, tag):
        for n in range(len(self.lines)):
            if self.lines[n] == tag:
                self.touch_block(n)
                
    def get_tag_state(self, tag):
        for l in self.lines:
            if tag == l.tag:
                return l.state
        return I
    
    def change_tag_state(self, tag, state):
        for n in range(len(self.lines)):
            if self.lines[n].tag == tag:
                self.lines[n].state = state
    
    def add_block(self, tag, state):
        for n in range(len(self.lines)):
            if self.lines[n].lru_bit == 0:
                self.lines[n].state = state
                self.lines[n].tag = tag
                self.touch_block(n)
            
                            
class Line:
    def __init__(self, tag, state):
        self.tag = tag
        self.state = state
        self.lru_bit = 0


class L2:
    def __init__(self, l2_size, block_size):
    
        self.index_pos = int(math.log(32, 2))
        self.tag_pos = self.index_pos + int(math.log(l2_size/(block_size), 2))
        
        self.cache_sets = {}
        
        self.misses = 0
        self.hits = 0
        
    def get_state(self, addr):
        idx = self.get_idx(addr)
        tag = self.get_tag(addr)
        
        if idx in self.cache_sets:
            if self.cache_sets[idx].tag == tag:
                return self.cache_sets[idx].state
        return I
            
   
    def bring_set(self, addr):
        idx = self.get_idx(addr)
        tag = self.get_tag(addr)
        
        self.cache_sets[idx] = Line(tag, 1)
    
    def get_tag(self, addr):
        return addr[self.tag_pos:]
        
    def get_idx(self, addr):
        return addr[self.index_pos:self.tag_pos]
        
class L1:
    def __init__(self, l1_size, block_size):
        self.hits = 0
        self.misses = 0
        self.accesses = 0
        self.invalidations = 0
        
        self.index_pos = int(math.log(32, 2))
        self.tag_pos = self.index_pos + int(math.log(l1_size/(2*block_size), 2))
        
        self.cache_sets = {}
        
        self.lo = None
        
        
    def set_lo(self, lo):
        self.lo = lo
    
    def get_state(self, addr):
        """
        State of data M,E,S,I.
        """
        idx = self.get_idx(addr)
        tag = self.get_tag(addr)
        
        if idx in self.cache_sets:
            return self.cache_sets[idx].get_tag_state(tag)
        else:
            return I
    
    def change_state(self, addr, state):
        idx = self.get_idx(addr)
        tag = self.get_tag(addr)
        
        self.cache_sets[idx].change_tag_state(tag, state)
        
    def invalidate(self, addr):
        idx = self.get_idx(addr)
        tag = self.get_tag(addr)
        
        self.cache_sets[idx].change_tag_state(tag, I)
        
    def bring_block(self, addr, state):
        idx = self.get_idx(addr)
        tag = self.get_tag(addr)
        
        if not (idx in self.cache_sets):
            self.cache_sets[idx] = SetLRU()
        self.cache_sets[idx].add_block(tag, state)
        
    def process_req(self, rw, addr):
        idx = self.get_idx(addr)
        tag = self.get_tag(addr)
        
        self.accesses += 1
        # obtiene estado del bloque en cache local
        state = self.get_state(addr)
        
        # operacione de lectura
        if rw==READ_REQ:
            if state:
                # there is a hit for load operation
                self.hits +=1
                self.cache_sets[idx].touch_tag(tag)
            else:
                self.misses +=1
                # there is no valid copy in L1
                # check in neighbor L1
                o_state = self.lo.get_state(addr)
                
                if o_state==E:
                    # change state to shared
                    self.lo.change_state(addr, S)
                    self.bring_block(addr, S)
                    
                elif o_state==S:
                    self.bring_block(addr, S)
                    
                elif o_state==M:
                    # change state to shared
                    self.lo.change_state(addr, S)
                    self.bring_block(addr, S)
                    
                else:
                    # not in neighbor, check in L2
                    self.bring_block(addr, E)
                    l2_state = l2.get_state(addr)
                    if l2_state:
                        # hit in L2
                        l2.hits += 1
                        
                    else:
                        # miss in L2, bring data from mem
                        l2.misses += 1
                        l2.bring_set(addr)
                     
        # operacion de escritura 
        else:
            # hacer cambios al estado local y snoop si es necesario
            if state == E:
                self.change_state(addr, M)
                self.cache_sets[idx].touch_tag(tag)
                
            elif state == S:
                # invalidate copy in other cache
                self.invalidations += 1
                self.change_state(addr, M)
                self.lo.invalidate(addr)
                self.cache_sets[idx].touch_tag(tag)
                
            elif state == M:
                self.cache_sets[idx].touch_tag(tag)
                
            else:
                self.misses +=1
                # there is no valid copy in L1
                
                # check neighbor
                o_state = self.lo.get_state(addr)
                
                if o_state in [E,S,M]:
                    self.invalidations += 1
                    self.lo.invalidate(addr)
                
                self.bring_block(addr, M)
                # and not in neighbor, check in L2
                l2_state = l2.get_state(addr)
                
                if l2_state:
                    # hit in L2
                    l2.hits += 1
                else:
                    # miss in L2, bring data from mem
                    l2.misses +=1
                    l2.bring_set(addr)
        
    def get_tag(self, addr):
        return addr[self.tag_pos:]
        
    def get_idx(self, addr):
        return addr[self.index_pos:self.tag_pos]


l2 = L2(l2_size, block_size)

p1 = L1(l1_size, block_size)
p2 = L1(l1_size, block_size)

p1.set_lo(p2)
p2.set_lo(p1)


access_count = 1

while(True):
    try:
        line = input()
        ls = int(line[2]) #un valor de cero indica un load y un uno un store
        addr = line[4:12]
        #print([line], ls, addr)
        
        # numero de acceso
         
        # procesa linea
        if((access_count % 4) == 0):
            p1.process_req(ls, addr)
        else:
            p2.process_req(ls, addr)
        
        access_count += 1 
            
    except EOFError:
        print("Done")
        break

# imprimir resultados
print(HL)
print("Miss rate global:\t\t\t\t%.5f\n"
        "Miss rate L1 CPU1:\t\t\t\t%.5f\n"
        "Miss rate L1 CPU2:\t\t\t\t%.5f\n"
        "Invalidaciones por coherencia CPU1:\t\t%5d\n"
        "Invalidaciones por coherencia CPU2:\t\t%5d\n" %
        (
        float(l2.misses)/float(p1.accesses+p2.accesses),
        float(p1.misses)/float(p1.accesses),
        float(p2.misses)/float(p2.accesses),
        p1.invalidations,
        p2.invalidations
        )
    )

print(HL)



