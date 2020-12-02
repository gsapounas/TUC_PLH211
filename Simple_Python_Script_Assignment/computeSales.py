#!/usr/bin/python

struct_list = []            #Create Struct list to store the receipts
dict1 = {'afm':0,'total':0}         #Dictionary used in question 2
dict2 = {'product':0,'total':0}         #Dictionary  used in question 3

def myFunc1(e):
  return e['afm']

def myFunc2(e):
  return e['product']


def print_menu():       
    print("Give your preference: (1: read new input file, 2: print statistics for a specific product, 3: print statistics for a specific AFM, 4: exit the program)")

loop = True
int_choice = -1

while loop:          
    print_menu()    
    choice = input()
    if choice == '1':
        int_choice = 1
        input_file = input("Enter the input file name: ")
        try:

            f = open(input_file, "r", encoding='utf-8')         #If file exists 

        except:

            print ("File not found")            #If file doesn't exist 
            continue

        temp_list = []
        x = 0
        
        

        for line in f:
                
                
            if line.startswith('-'):            #If line starts with "-"
                if temp_list:
                    total_total = 0             #Total sum initialize        
                    last_index = len(temp_list)-1       #Last index of receipt
                    temp = temp_list[0].split()         
                    afm = temp[1]           
                    #afm_int = int(afm)
                    
                    if len(afm) == 10:          #If afm length=10 then afm is valid
                        
                        temp = temp_list[-1].split()
                        total = temp[1]
                        total_float = float(total)
                        
                        i = 1
                        products = []
                        while i < len(temp_list)-1:     #Loop through the products and do calculations
                            temp = temp_list[i].split()
                            product = temp[0]
                            product = product[:-1]
                            
                            quantity = temp[1]
                            price = temp[2]
                            total_price = temp[3]

                            quantity_int = int(quantity)
                            price_float = float(price)
                            total_price_float = float(total_price)
                            result = quantity_int * price_float

                            if round(result,2) == round(total_price_float,2):         #Checks if total price of products is right
                                total_total = total_total+total_price_float
                                
                                product = product.upper()
                                if product not in products:         #Checks if product doesn't exist in product list
                                    products.append(product)
                                    products.append(total_price_float)
                                    
                                else:
                                    
                                    products[1] = products[1]+total_price_float
                                
                            else:
                                
                                break

                                
                            i += 1              
                        
                        if round(total_total,2) == round(total_float,2):          #checks if total sum is total sum of product total sum
                            

                                
                            if afm not in struct_list:          #if afm is not on struct list
                                struct_list.append(afm) 
                                struct_list.append(products)
                                
                            else:                              #if afm is on struct list it makes calculations
                                
                                j = struct_list.index(afm)      #finds in which indes afm is stored
                                temp = struct_list[j+1]
                                
                                for idx1, val1 in enumerate(products):
                                       
                                    if idx1%2 == 0:
                                        for idx2, val2 in enumerate(temp):
                                            
                                            if idx2%2 == 0:
                                                if val1 == val2:
                                                    temp[idx2+1] = temp[idx2+1] + products[idx1+1] 
                                k = 0
                                for i in products[::2]:
                                    if i not in temp:
                                        temp.append(i)
                                        a = products[k+1]
                                        
                                        temp.append(a)
                                    k = k+2
                                                    


                                                    
                                                    

    
                                    
                        temp_list = []          
                        x = 0 
                    
                    else:
                        temp_list = []
                        x = 0


            else:
                temp_list.append(line.upper())          #list that stores input lines
                x = x+1
                
        f.close()


    elif choice == '2':
        int_choice = 2
        list1 = []
        input_product = input("Give product name: ")
        input_product = input_product.upper()           #makes product name upper
        total_per_product = 0
        for idx1, val1 in enumerate(struct_list):           #runs the struct list and checks if index mod 2 is equal to 0 so we can get the afm value
            if idx1%2 == 0:
                afm = val1
                
            else:
                
                if input_product in struct_list[idx1]:          #checks if input product exists in struct list 
                    for idx2, val2 in enumerate(struct_list[idx1]):         #runs the product list which is stored in every index that idx mod 2 != 0
                        if idx2%2 == 0:             #if idx2 mod 2 == 0 then we have the product name 
                            if input_product == val2:
                                total_per_product = struct_list[idx1][idx2+1]
                                dict1['afm'] = afm          #stores afm in dict1 value with the key "afm" 
                                dict1['total'] = ("%.2f" % round(total_per_product,2))          #stores total_per_product in dict1 value with the key "total" 
                                list1.append(dict1.copy())          #appends to list1 the content of dict1
        

        list1.sort(key = myFunc1)               #sorts list1 based on the key inside myfunc1 (afm)
        
        for i in list1:
            print(i['afm'],i['total'])          #prints value of "afm" key and value of "total" key


        
    elif choice == '3':
        int_choice = 3
        list2 = []
        input_afm = input("Give AFM: ")
        total_per_product = 0
        if input_afm in struct_list:            #checks if input afm exists in struct list 
            for idx1, val1 in enumerate(struct_list):           #this happens with the same logic as in the queston 2 
                if idx1%2 == 0:
                    afm = val1
                else:
                    if afm == input_afm:
                        
                        for idx2, val2 in enumerate(struct_list[idx1]):         
                            if idx2%2 == 0:   
                                total_per_product = struct_list[idx1][idx2+1]
                                dict2['product'] = val2         #stores val2 in dict2 value with the key "product" 
                                dict2['total'] = ("%.2f" % round(total_per_product,2))          #stores total_per_product in dict2 value with the key "total" 
                                list2.append(dict2.copy())      #appends to list2 the content of dict2
        

        list2.sort(key = myFunc2)           #sorts list2 based on the key inside myfunc2 (product)

        for i in list2:
            print(i['product'],i['total'])          #prints value of "product" key and value of "total" key
            
    elif choice == '4':
        int_choice = 4
        loop = False  


