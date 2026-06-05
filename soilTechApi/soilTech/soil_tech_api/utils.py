# {'crop': 'maize', 'nitrogen': 8, 'phosphorus': 66, 'potassium': 58, 'pH': 8.1}

# compute mineral fertilzers 

CNSD={'crop' :['n','p','k']} # CNSD--> crop nutrient sufficiency data

"""
    cnsd={crop :[n,p,k]}
"""
def compute_mineral_fertilizer(data, CNSD):
    pass
    """
    ----Nitrogen----
    The crop N requirement as obtained from crop N response curves is used.
    as soil N content in the soil are variable.

    ----Potassium & Phosphorous----
    The Sufficiency Maintance Approach is used. 
    *Base values for the computation under the sufficiency maintance approach.
    -Bulk density
    -

    """
    #Selected crop and soil test values
    crop=data['crop']
    N_test_value=data['nitrogen']
    K_test_value=data['potassium']
    P_test_value=data['phosphorus']

    #crop nutrient sufficency data
    N_sufficiency=CNSD[crop][0]
    P_sufficiency=CNSD[crop][1]
    K_sufficiency=CNSD[crop][2]

    # critical values
    N,P,K=[,2,0.42]